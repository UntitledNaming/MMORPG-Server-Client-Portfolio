#include <iostream>
#include <windows.h>
#include "TextParser.h"
#pragma warning(disable:4996)
// ���ڷ� ���޵� �����̸��� �����ϰ� �޸� �����Ҵ��ؼ� ������ �����͸� ���� �޸𸮿� �ű�� ���

Parser::Parser()
{

}

Parser::~Parser()
{
	free(_pTextBuffer);
}

bool Parser::LoadFile(const char* FileName)
{
	strcpy(_FileName, FileName);

	int FILESIZE = 0;
	FILE* fp;
	errno_t err;

	err = fopen_s(&fp, _FileName, "rt");
	if (err != 0)
	{
		std::cout << "file stream not open" << std::endl;
		return false;
	}

	fseek(fp, 0, SEEK_END);
	FILESIZE = ftell(fp);
	_pTextBuffer = (char*)malloc(FILESIZE);
	_pRead = _pTextBuffer;
	fseek(fp, 0, SEEK_SET);
	fread(_pTextBuffer, FILESIZE, 1, fp);

	fclose(fp);

	return true;
}

//pRead�� ���� �ʿ� ���� ���ڿ� ����� �ּ� �ǳ� �ٰ� �о �Ǵ� �ּҷ� �ű��
//���� �ʿ� ���� ���ڿ� : ����, �ּ�, ����
bool Parser::SkipNoneWord()
{
	while (1)
	{
		//���ۿ� ����� ������ ����� �ּҴ� �ǳ� �ٱ�
		if (*_pRead == ' ' || *_pRead == 0x0a || *_pRead == 0x09 || *_pRead == ';')
		{
			_pRead++;
			continue;
		}

		// /���ڸ� ������ ���� ���ڰ� /Ȥ�� *�̸� �ּ����� ó��, �ּ��� �ƴϸ� �ݺ��� Ż��
		else if (*_pRead == '/')
		{
			char* temp = _pRead;
			temp++;

			//  //������ �ּ��̴� pRead�� �Űܾ� ��. //�� ������ ������ �� ���� �ּҷ� �Ű��ָ� ��. �ű�� �ٽ� �ݺ��� ó������ ���ư��� ����, ������� �˻�
			if (*temp == '/')
			{
				_pRead = temp;
				while (1)
				{
					_pRead++;
					if (*_pRead == 0x0a)
					{
						_pRead++;
						break;
					}
				}
				continue;
			}

			// /**/������ �ּ��̴� */������ / ���� �ּҷ� _pRead �ű�� �ٽ� �ݺ��� ó������ ���ư��� ����, ������� �˻�
			else if (*temp == '*')
			{
				_pRead = temp;
				while (1)
				{
					_pRead++;
					if (*_pRead == '*')
					{
						temp = _pRead;
						temp++;
						if (*temp == '/')
						{
							_pRead = temp;
							_pRead++;
							break;
						}

					}
				}
				continue;
			}

		}

		//����, ����, �ּ��� �ƴ϶�� _pRead �״�� �ΰ� �Լ� Ż��
		else
			return true;


	}
	return false;
}

//���ۿ� ����� ���ڿ� �߿� �ܾ�� ������ ������ �ּҸ� �˷��ֱ� ���� �ּҰ��� �����ϴ� char*�� �ּҰ��� ����.
//�� �ܾ��� ���̰� ������ 
bool Parser::GetNextWord(char** chppBuffer, int* lpLength)
{
	int count = 0;
	bool Start = false;

	SkipNoneWord();

	while (1)
	{
		if (_pRead == nullptr)
			return false;

		//�ܾ��� ���� ��ġ���� �д� �����͸� ��� �̵� ��Ű�µ� �Ʒ��� ���� ���ڸ� ������ �ܾ ���� ������ �ν�
		if (*_pRead == ',' || *_pRead == 0x20 || *_pRead == 0x08 || *_pRead == 0x09 || *_pRead == 0x0a
			|| *_pRead == 0x0d || *_pRead == ';')
		{
			break;
		}
		
		else if (*_pRead == '"')
		{
			if (!Start)
			{
				Start = true;
				_pRead++;
				count++;
				continue;
			}

			_pRead++;
			count++;
			break;
		}

		_pRead++;
		count++;
	}

	if (Start)
	{
		*chppBuffer = _pRead - count + 1;

		*lpLength = count - 2;
	}
	else
	{
		*chppBuffer = _pRead - count;
		*lpLength = count;
	}

	return true;
}

//��ü�� �����ϰ� ������ Ŭ�������� ���� ���� ������ ����� �ֱ�
bool Parser::GetValue(const char* szName, int* pValue)
{
	_pRead = _pTextBuffer; // Value�� ���� �� ���� ��ü �޸� ���۸� ó������ Ž��
	char* chpBuff;
	char chWord[256];
	int iLength;

	//� �ܾ �߰ߵ� �� while�� ������ ����
	while (Parser::GetNextWord(&chpBuff, &iLength))
	{
		//chWord �迭 0���� �ʱ�ȭ
		memset(chWord, 0, 256);

		//chWord �迭�� chpBuff �����Ͱ� ����Ű�� ���ں��� iLength ��ŭ �ű�. 
		memcpy(chWord, chpBuff, iLength);

		//ã���� �ϴ� ���ڿ��� �߰��� �ܾ �����ϴٸ�
		if (strcmp(szName, chWord) == 0)
		{
			// � �ܾ �߰��� ������_pRead �̵�
			if (Parser::GetNextWord(&chpBuff, &iLength))
			{
				memset(chWord, 0, 256);
				memcpy(chWord, chpBuff, iLength);
				//�߰��� ���ڰ� =�϶�
				if (strcmp(chWord, "=") == 0)
				{
					if (GetNextWord(&chpBuff, &iLength))
					{
						memset(chWord, 0, 256);
						memcpy(chWord, chpBuff, iLength);
						*pValue = atoi(chWord);

						return true;
					}
					return false;
				}
			}
			return false;
		}
	}
	return false;
}

// ��ȯ ���ڿ� NULL ���� �� Len�� NULL ����
bool Parser::GetValue(const char* szName, st_Msg* pMsg)
{
	_pRead = _pTextBuffer; // Value�� ���� �� ���� ��ü �޸� ���۸� ó������ Ž��
	char* chpBuff;
	char chWord[256];
	int iLength;

	//� �ܾ �߰ߵ� �� while�� ������ ����
	while (Parser::GetNextWord(&chpBuff, &iLength))
	{
		//chWord �迭 0���� �ʱ�ȭ
		memset(chWord, 0, 256);
		//chWord �迭�� chpBuff �����Ͱ� ����Ű�� ���ں��� iLength ��ŭ �ű�. 
		memcpy(chWord, chpBuff, iLength);

		//ã���� �ϴ� ���ڿ��� �߰��� �ܾ �����ϴٸ�
		if (strcmp(szName, chWord) == 0)
		{
			// � �ܾ �߰��� ������_pRead �̵�
			if (Parser::GetNextWord(&chpBuff, &iLength))
			{
				memset(chWord, 0, 256);
				memcpy(chWord, chpBuff, iLength);
				//�߰��� ���ڰ� =�϶�
				if (strcmp(chWord, "=") == 0)
				{
					if (Parser::GetNextWord(&chpBuff, &iLength))
					{

						//ã�� ���ڿ��� �ּҸ� �ƿ��Ķ���Ϳ� ��Ƽ� �ܺο� �ѱ�.
						memcpy(pMsg->s_ptr, chpBuff, iLength); // �ǵ� " ���� �� ����
						*(pMsg->s_ptr + iLength) = NULL;
						pMsg->s_len = iLength;
						return true;
					}
				}
			}
		}
	}
	return false;
}


