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

    if (Edit_AccountID)
        Edit_AccountID->SetKeyboardFocus();
}

void UM1LoginWidget::OnClickLogin()
{
    if (!NetworkManager)
        return;

    uint64 AccountID = 0;
    if (!ParseAccountID(AccountID))
    {
        if (Text_Message)
        {
            Text_Message->SetText(FText::FromString(TEXT("Account ID를 입력하세요.")));
            Text_Message->SetVisibility(ESlateVisibility::Visible);
        }
        return;
    }

    CMessage* Msg = CMessage::Alloc();
    Msg->Clear(1);

    *Msg << AuthProtocol::PACKET_CS_GAME_LOGIN_REQ;
    *Msg << AccountID;

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

bool UM1LoginWidget::ParseAccountID(uint64& OutAccountID) const
{
    if (!Edit_AccountID)
        return false;

    FString Text = Edit_AccountID->GetText().ToString();
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

