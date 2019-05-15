#include "client_application.h"
#include "osgresourcemanager.h"
#include "ballooncontroller.h"
#include "avatarcontroller.h"
#include <iostream>

void MyInteractionCallback(unsigned int interactionID, const TA_ParamList& parameters)
{
	TA_Resource* pInteraction = TA_ResourceManager::GetInstance()->FindResourceByID(interactionID, TA_Resource::PATTERN);
	if (pInteraction->GetName() == "Interaction1")
	{
		int& p1 = *((int*)parameters[0]);
		double& p2 = *((double*)parameters[1]);
		float& p3 = *((float*)parameters[2]);
		std::cout << "Interaction 1 received with parameters " << p1 << p2 << p3 << std::endl;
	}
	else if (pInteraction->GetName() == "Interaction2")
	{
		int& p1 = *((int*)parameters[0]);
		int& p2 = *((int*)parameters[1]);
		std::cout << "Interaction 2 received " << p1 << " " << p2 << std::endl;
	}
	else if (pInteraction->GetName() == "My Interaction")
	{
		TA_Point3D& p1 = *((TA_Point3D*)parameters[0]);
		TA_Point3F& p2 = *((TA_Point3F*)parameters[1]);
		std::cout << "My Interaction received " << p1.Length() << " " << p2.Length() << std::endl;

	}
}

ClientApplication::ClientApplication() :
	m_pWorld(NULL)
	, m_pRenderer(NULL)
	, m_pTimer(NULL)
	, m_pResourceManager(TA_ResourceManager::GetInstance())
	, m_bConsoleMode(false)
{
	strcpy(m_pczSrvIP, "127.0.0.1");
}

bool ClientApplication::_LoadSettings()
{
	FILE* pSettings = fopen("cl_settings.txt", "r");

	if (pSettings)
		fscanf(pSettings, "Server IP = %s\n", m_pczSrvIP);
	else
		return false;
	fclose(pSettings);
	return true;
}

bool ClientApplication::Init(bool bConsoleMode)
{
	if (!_LoadSettings())
		return false;

	m_bConsoleMode = bConsoleMode;

	m_pResourceManager->Init(TA_CLIENT, !m_bConsoleMode ? new OSGResourceManagerImpl() : NULL);
	m_pResourceManager->RegisterController(new BalloonController);

	TA_StateAttribute * pAttr = NULL;
	// Get a virtual environment instance
	m_pWorld = m_pResourceManager->GetWorld();

	TA_NetworkInterface * pNetInterface = m_pResourceManager->GetNetworkInterface();
	pNetInterface->GetCurProcessID();
	// Connect our process to the server
	if (!pNetInterface->Connect(m_pczSrvIP, true))
		return false;

	// Subscribe on VE objects state updates
	pNetInterface->SubscribePattern("Interaction1");
	pNetInterface->SubscribePattern("Interaction2");
	pNetInterface->SubscribePattern("My Interaction");
	pNetInterface->SetInteractionHandleCallback(MyInteractionCallback);

	// Create window
	TA_Window * pWindow = m_pResourceManager->CreateGraphicWindow();
	pWindow->Resize(100, 100, 640, 480);

	// Create camera
	TA_Camera * pCamera = m_pResourceManager->CreateCamera();
	pCamera->SetWindow(pWindow);
	pCamera->SetViewPort(0, 0, 640, 480);
	pCamera->SetFrustrumParameters(45.0, 0.1, 1000.0);
	pCamera->LookAt(TA_Point3D(100.0, 100.0, 20.0), TA_Point3F(-TA_PI_0_5, 0.0f, -2.50f));
	m_pWorld->SetCamera(pCamera);

	// Create renderer
	m_pRenderer = m_pResourceManager->GetRenderer();
	m_pRenderer->SetSceneData(m_pWorld);
	// Call Realize() after setting windows and cameras
	m_pRenderer->Realize();

	// Add our part to world
	// Here we create an object, first locally, and after this share it
	// NOTE: if object is already created, next call will be ignored and NULL returned
	pBalloon = m_pResourceManager->CreateObject("My balloon", true, false);
	if (pBalloon)
	{
		TA_State* pState = pBalloon->GetState();
		// Load and set object graphic representation
		TA_Model* pBalloonModel = m_pResourceManager->CreateModel("Balloon.3ds");
		pBalloon->SetModel(pBalloonModel);

		pAttr = pState->GetAttribute("POSITION");
		pAttr->SetSerializationMode(TA_StateAttribute::SERIALIZE_WITH_CONSTANT_RATE); //грубо говоря частота обновления у других участников
		pAttr->SetSerializationRate(1.0);											//например на стороне сервера.
		pAttr->EnableFiltering(false);				//при false - обновление получается таким, как будто рывки
													//при true - эти рывки не становятся столь "квадратными". То есть получается "сглаженный рывок"
	//	pAttr->SetSerializationRate(45.0);											
	//	pAttr->EnableFiltering(false);				
		//pAttr->SetSerializationMode(TA_StateAttribute::SERIALIZE_ON_EVERY_CHANGE);
		//pAttr->SetSerializationRate(5.0);

		pAttr = pState->GetAttribute("SCALE");
		pAttr->SetValue(TA_Point3F(4.0f)); //x = y = z = число. масштаб по осям относительно исходного размера по идеи

		// Создание контроллера ранее зарегестированного и его назначение на объект
		BalloonController* balcon = new BalloonController();
		TA_Controller* pController = balcon;
		pBalloon->SetController(pController);
		pController->Enable();

		// Add object to scene locally
		m_pWorld->GetRootObject()->AddChild(pBalloon);

		// Share object, that it becomes available for every process
		pBalloon->Share();
	}

	pAvatar = m_pResourceManager->CreateObject("My Avatar", true, false);
	if (pAvatar)
	{
		TA_State* pState = pAvatar->GetState();

		TA_Model* pAvatarModel = m_pResourceManager->CreateModel("icosahedron.osg");
		pAvatar->SetModel(pAvatarModel);

		pAttr = pState->GetAttribute("POSITION");
		pAttr->SetSerializationMode(TA_StateAttribute::SERIALIZE_WITH_CONSTANT_RATE);
		pAttr->SetSerializationRate(10.0);
		pAttr->EnableFiltering(true);
		pAttr = pState->GetAttribute("SCALE");
		pAttr->SetValue(TA_Point3F(4.0f));

		//тоже, что и для шара, только для него //вопрос про регистрацию
		AvatarController* avacon = new AvatarController();
		TA_Controller* pController = avacon;
		avacon->pCam = pCamera;		//положение камеры аватара берем в качестве исходного от главного
									//движение можно выполнять кнопками главной камеры WASD
		pAvatar->SetController(pController);
		pController->Enable();

		m_pWorld->GetRootObject()->AddChild(pAvatar);

		pAvatar->Share();
	}


	// Create timer
	m_pTimer = m_pResourceManager->GetTimer();

	// Sync clocks with server
	m_pTimer->Sync();

	// Register our own interaction pattern
	TA_Pattern* pInteraction3 = new TA_Pattern(TA_Pattern::INTERACTION_PATTERN);
	pInteraction3->SetName("My Interaction");
	pInteraction3->AllocateParameter("pos", TA_StateAttribute::VECTOR_3D);
	pInteraction3->AllocateParameter("rot", TA_StateAttribute::VECTOR_3F);
	m_pResourceManager->RegisterPattern(pInteraction3);

	return true;
}

void ClientApplication::Run()
{
	Tick oldTick = m_pTimer->GetTick();

	TA_StringList attrList;
	attrList.push_back("POSITION");
	attrList.push_back("ROTATION");

	m_pWorld->SetCycleTime(1.0 / 60.0, true);

	int iRnd = 0;
	int iToggle = 0;
	TA_Key key;
	if (m_bConsoleMode)
	{
		std::cout << "TerraNet Client v. 1.054 is started..." << std::endl << "Entering console mode" << std::endl;
		while (true)
			m_pWorld->Simulate();
	}
	else
	{
		while (!m_pRenderer->Done())
		{
			double dCurTime = m_pTimer->GetTime();
			double dStartTime = dCurTime + 5.0;
			if (m_pResourceManager->GetInputManager()->IsKeyPressed())
			{
				key = m_pResourceManager->GetInputManager()->GetKey();
				switch (key.code)
				{
				case 'p':
				{
					TA_Object* object = m_pWorld->GetUserObject();
					if (!object)
						break;
					object->PlayStateTrack((object->GetName() + "_local.track").c_str(), dStartTime);
				} break;
				case 'r':
				{
					TA_Object* object = m_pWorld->GetUserObject();
					if (!object)
						break;
					object->RecordStateTrack((object->GetName() + "_local.track").c_str(), dStartTime, dStartTime + 10.0, attrList);
				} break;
				case 'q':
				{
					TA_ParamList paramList;
					if (++iRnd % 3 == 0)
					{
						int param1 = 1;
						double param2 = 2.0;
						float param3 = 5.3;
						paramList.push_back(&param1);
						paramList.push_back(&param2);
						paramList.push_back(&param3);
						// Send interaction to everyone who subscribed it
						m_pResourceManager->GetNetworkInterface()->SendInteraction("Interaction1", paramList, LOW_RELIABILITY);
					}
					else {
						if (iRnd % 3 == 1)
						{
							int param1 = 1;
							int param2 = 15;
							paramList.push_back(&param1);
							paramList.push_back(&param2);
							// Send interaction to everyone who subscribed it
							m_pResourceManager->GetNetworkInterface()->SendInteraction("Interaction2", paramList, HIGH_RELIABILITY);
						}
						else {
							TA_Point3D param1 = TA_Point3D(20, -11, 7);
							TA_Point3F param2 = TA_Point3F(2.0f, 10.0f, 25.0f);
							paramList.push_back(&param1);
							paramList.push_back(&param2);
							m_pResourceManager->GetNetworkInterface()->SendInteraction("My Interaction", paramList, HIGH_RELIABILITY);
						}
					}
				} break;
				}
			}
			m_pWorld->Simulate();
			m_pRenderer->RenderFrame();
		}
	}
}

void ClientApplication::Finalize()
{
	// Here we remove our objects from world if they are not captured by other users
	m_pResourceManager->RemoveObject((TA_Object*)m_pResourceManager->FindResourceByName("My balloon", TA_Resource::OBJECT));
	m_pResourceManager->RemoveObject((TA_Object*)m_pResourceManager->FindResourceByName("My Avatar", TA_Resource::OBJECT));

}

