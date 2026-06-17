
#include <windows.h>
#include <iostream>
#include <string>
#include <mysql.h>
#include <strsafe.h>
#include <unordered_map>
#include <errmsg.h>
#include <mysqld_error.h>
#include "LogClass.h"
#include "DBTLS.h"


DBTLS::DBTLS(const CHAR* DBip, INT DBPort, std::string& schema) 
{
	m_TlsIdx = TlsAlloc();
	if (m_TlsIdx == TLS_OUT_OF_INDEXES)
	{
		__debugbreak();
	}

	m_Schema = schema;
	m_DBIP = DBip;
	m_DBPort = DBPort;
	m_DBQArrayIdx = DBTLS_IDX;

	mysql_library_init(0, NULL, NULL);
}

DBTLS::~DBTLS()
{
	// 쿼리 배열 0번째 index
	for (int i = 0; i <= m_DBQArrayIdx; i++)
	{
		if (m_DBQueryAry[i])
		{
			delete m_DBQueryAry[i];
			m_DBQueryAry[i] = nullptr;
		}
	}
}

bool DBTLS::DB_Post_Query(DB_QUERY_RESULT& Result, const CHAR* QueryString, ...)
{
	DB_Query* ret = nullptr;
	INT16     retIDX;

	ret = (DB_Query*)TlsGetValue(m_TlsIdx);
	if (ret == nullptr)
	{
		// 관리 배열범위 체크
		retIDX = InterlockedIncrement16(&m_DBQArrayIdx);
		if (retIDX >= DBTLS_MAX_COUNT)
		{
			InterlockedDecrement16(&m_DBQArrayIdx);
			return false;
		}

		// 쿼리 처음 호출
		ret = new DB_Query(this);


		TlsSetValue(m_TlsIdx, ret);

		// 관리 배열에 저장
		m_DBQueryAry[retIDX] = ret;

	}
	bool Success = false;
	va_list args;
	va_start(args, QueryString);
	Success = ret->DB_Post_Query(Result,QueryString, args);
	va_end(args);

	return Success;
}

MYSQL_RES* DBTLS::DB_GET_Result(int type)
{
	DB_Query* ret = nullptr;
	ret = (DB_Query*)TlsGetValue(m_TlsIdx);

	return ret->DB_GET_Result(type);
}

MYSQL_ROW* DBTLS::DB_Fetch_Row(MYSQL_RES* res)
{
	DB_Query* ret = nullptr;
	ret = (DB_Query*)TlsGetValue(m_TlsIdx);
	return ret->DB_Fetch_Row(res);
}

void DBTLS::DB_Free_Result()
{
	DB_Query* ret = nullptr;
	ret = (DB_Query*)TlsGetValue(m_TlsIdx);
	ret->DB_Free_Result();
}

DBTLS::DB_Query::DB_Query(DBTLS* parent) : m_Parent(parent), m_sql_result(nullptr), m_sql_row(nullptr)
{
	mysql_init(&m_Conn);

	m_Connection = mysql_real_connect(&m_Conn, m_Parent->m_DBIP.c_str(), "root", "1q2w3e4r", m_Parent->m_Schema.c_str(), m_Parent->m_DBPort, (char*)NULL, 0);
	if (m_Connection == NULL)
	{
		LOG(L"DB", en_LOG_LEVEL::dfLOG_LEVEL_ERROR, L"DB Connect Error... / UniqID : %S ", mysql_error(&m_Conn));
		__debugbreak();
	}

}

DBTLS::DB_Query::~DB_Query()
{
	mysql_close(m_Connection);
}

bool DBTLS::DB_Query::ReConnect()
{
	mysql_close(m_Connection);

	mysql_init(&m_Conn);

	m_Connection = mysql_real_connect(&m_Conn, m_Parent->m_DBIP.c_str(), "root", "1q2w3e4r", m_Parent->m_Schema.c_str(), m_Parent->m_DBPort, (char*)NULL, 0);
	if (m_Connection == NULL)
	{
		LOG(L"DB", en_LOG_LEVEL::dfLOG_LEVEL_ERROR, L"DB Connect Error... / UniqID : %s ", mysql_error(&m_Conn));
		return false;
	}
	return true;
}

bool DBTLS::DB_Query::DB_Post_Query(DB_QUERY_RESULT& Result, const CHAR* QueryString, const va_list& args)
{
	INT     query_stat;
	HRESULT ret;
	CHAR    pBuffer[DBQUERY_DEFAULT_LEN];
	BOOL    reSize = false;

	Result = DB_QUERY_RESULT::None;

    ret = StringCchVPrintfA(pBuffer, DBQUERY_DEFAULT_LEN, QueryString, args);
    
    // 쿼리 스트링 길이가 할당 크기보다 크면 중단
	if (ret == STRSAFE_E_INSUFFICIENT_BUFFER)
		return false;

	query_stat = mysql_query(m_Connection, pBuffer);
	if (query_stat != 0)
	{
		int error_code = mysql_errno(m_Connection);
		LOG(L"DB",en_LOG_LEVEL::dfLOG_LEVEL_ERROR,L"DB mysql_query Error : %s" ,mysql_error(&m_Conn));

		// 재연결 1번 시도 후 쿼리 날리기
		if (error_code == CR_SERVER_GONE_ERROR || error_code == CR_SERVER_LOST)
		{
			// 재연결 후 쿼리 성공하면 true 리턴
			if (ReConnect() && mysql_query(m_Connection, pBuffer) == 0)
			{
				Result = DB_QUERY_RESULT::Success;
				return true;
			}

			Result = DB_QUERY_RESULT::ConnectLost;
			return false;
		}

		// 쿼리문 이상한것이면다 크래쉬
		else if (error_code == ER_PARSE_ERROR || error_code == ER_NO_SUCH_TABLE || error_code == ER_BAD_FIELD_ERROR)
			__debugbreak();

		// 쿼리에 담긴 데이터 문제(이름 중복, UID 중복 등)
		else if (error_code == ER_DUP_ENTRY || error_code == ER_BAD_NULL_ERROR
			|| error_code == ER_NO_REFERENCED_ROW_2 || error_code == ER_ROW_IS_REFERENCED_2)
		{
			Result = DB_QUERY_RESULT::Constraint;
			return false;
		}

		// 그외 버그는 false 리턴
		return false;
	}

	Result = DB_QUERY_RESULT::Success;
	return true;
}

MYSQL_RES* DBTLS::DB_Query::DB_GET_Result(int type)
{
	if (type == 0)
	{
		m_sql_result = mysql_store_result(m_Connection);
		return m_sql_result;
	}

	else if (type == 1)
	{
		m_sql_result = mysql_use_result(m_Connection);
		return m_sql_result;
	}


	return nullptr;
}

MYSQL_ROW* DBTLS::DB_Query::DB_Fetch_Row(MYSQL_RES* res)
{
	if (m_sql_result != res)
		return nullptr;

	m_sql_row = mysql_fetch_row(m_sql_result);
	if (m_sql_row == NULL)
	{
		return nullptr;
	}

	return &m_sql_row;
}

void DBTLS::DB_Query::DB_Free_Result()
{
	mysql_free_result(m_sql_result);
}
