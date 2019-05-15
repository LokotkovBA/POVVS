#include "avatarcontroller.h"

AvatarController::AvatarController() :
	m_pInputManager(TA_ResourceManager::GetInstance()->GetInputManager())
	, m_dOldTime(0)
{
	m_iType = MOVEMENT_CONTROLLER;
	SetName("AvatarController");
}

void AvatarController::Init(TA_Resource* pResource)
{
	// Note: Always call TA_Controller::Init() method to initialize TA_Controller class
	TA_Controller::Init(pResource);

	TA_State* state = m_pObject->GetState();
	m_pPosAttr = state->GetAttribute("POSITION");
	m_pRotAttr = state->GetAttribute("ROTATION");
}

void AvatarController::Update(double dTime)
{
	TA_Point3D pos;
	m_pPosAttr->GetValue(pos);
	double dt = dTime - m_dOldTime;
	pos = pCam->GetPosition();
	m_pPosAttr->SetValue(pos, dTime);
}
