#include "UI/M1RespawnWidget.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "Network/M1NetworkManager.h"
#include "Network\ClientCore/CMessage.h"
#include "ContentsProtocol.h"
#include "ContentsEnum.h"
#include "NetPacketHeader.h"

void UM1RespawnWidget::NativeConstruct()
{
    Super::NativeConstruct();

    NetworkManager = GetGameInstance()->GetSubsystem<UM1NetworkManager>();

    if (Button_Respawn)
        Button_Respawn->OnClicked.AddDynamic(this, &UM1RespawnWidget::OnClickRespawn);

    if (Text_Message)
        Text_Message->SetText(FText::FromString(TEXT("You Died")));
}

void UM1RespawnWidget::OnClickRespawn()
{
    if (!NetworkManager)
        return;

    if (Button_Respawn)
        Button_Respawn->SetIsEnabled(false);

    CMessage* Msg = CMessage::Alloc();
    Msg->Clear(1);

    *Msg << FieldProtocol::PACKET_CS_RESPAWN_REQ;

    NetworkManager->SendPacket(Msg, static_cast<uint8>(ERouteType::GROUP), ServiceID::NONE_SERVICE);

    CMessage::Free(Msg);
}
