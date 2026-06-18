#include "UI/M1LoginWidget.h"
#include "Components/EditableTextBox.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "Network/M1NetworkManager.h"
#include "Network\ClientCore/CMessage.h"
#include "ContentsProtocol.h"
#include "ContentsEnum.h"
#include "NetPacketHeader.h"

void UM1LoginWidget::NativeConstruct()
{
    Super::NativeConstruct();

    NetworkManager = GetGameInstance()->GetSubsystem<UM1NetworkManager>();

    if (Button_Login)
        Button_Login->OnClicked.AddDynamic(this, &UM1LoginWidget::OnClickLogin);

    if (Text_Message)
        Text_Message->SetVisibility(ESlateVisibility::Collapsed);

    if (Edit_CharacterID)
        Edit_CharacterID->SetKeyboardFocus();
}

void UM1LoginWidget::OnClickLogin()
{
    if (!NetworkManager)
        return;

    uint64 CharacterID = 0;
    if (!ParseCharacterID(CharacterID))
    {
        if (Text_Message)
        {
            Text_Message->SetText(FText::FromString(TEXT("Character ID를 입력하세요.")));
            Text_Message->SetVisibility(ESlateVisibility::Visible);
        }
        return;
    }

    char token[64] = {};
    CMessage* LoginReqMsg = CMessage::Alloc();
    LoginReqMsg->Clear(1);

    *LoginReqMsg << AuthProtocol::PACKET_CS_GAME_LOGIN_REQ;
    LoginReqMsg->PutData(token, sizeof(token));

    NetworkManager->SendPacket(
        LoginReqMsg,
        static_cast<uint8>(ERouteType::GROUP),
        ServiceID::NONE_SERVICE
    );

    CMessage::Free(LoginReqMsg);


    CMessage* Msg = CMessage::Alloc();
    Msg->Clear(1);

    *Msg << AuthProtocol::PACKET_CS_GAME_CHARACTER_SELECT;
    *Msg << CharacterID;

    NetworkManager->SendPacket(
        Msg,
        static_cast<uint8>(ERouteType::GROUP),
        ServiceID::NONE_SERVICE
    );

    CMessage::Free(Msg);

    if (Button_Login)
        Button_Login->SetIsEnabled(false);

    if (Text_Message)
    {
        Text_Message->SetText(FText::FromString(TEXT("접속 요청 중...")));
        Text_Message->SetVisibility(ESlateVisibility::Visible);
    }
}

bool UM1LoginWidget::ParseCharacterID(uint64& OutAccountID) const
{
    if (!Edit_CharacterID)
        return false;

    FString Text = Edit_CharacterID->GetText().ToString();
    Text.TrimStartAndEndInline();

    if (Text.IsEmpty())
        return false;

    for (const TCHAR Ch : Text)
    {
        if (!FChar::IsDigit(Ch))
            return false;
    }

    OutAccountID = FCString::Strtoui64(*Text, nullptr, 10);
    return OutAccountID > 0;
}

