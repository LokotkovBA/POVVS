#include <math.h>
#include "ballooncontroller.h"

#define M_PI 3.14159265358979323846	//����� ��������� ��� �������� ������������
#define g 9.8		//��������� ���������� �������
#define ro_air 1.2	//��������� �������
#define V 1000.0	//����� ����
#define massa 1200 //����� ���� (� �������, � ��������, ��������, ������)

BalloonController::BalloonController() :
	m_pInputManager(TA_ResourceManager::GetInstance()->GetInputManager())
	, m_dOldTime(0)
	, m_dThrustForce(0)
	, m_vel(0)
	, v(0)				//�������� ����
	, a(0)				//��������� ����
	, koef(1)			//������������� ��������� ����������� ������ ����
	, windForce(0)		//���� �����
	, angle(0)			//����������� �����
{
	m_iType = MOVEMENT_CONTROLLER; //��� ����������� - ��� �������� �������� �������
	SetName("BalloonController");
}

// Init controller
void BalloonController::Init(TA_Resource* pResource)
{
	// Note: Always call TA_Controller::Init() method to initialize TA_Controller class
	TA_Controller::Init(pResource);

	TA_State* state = m_pObject->GetState();
	m_pPosAttr = state->GetAttribute("POSITION");
	m_pRotAttr = state->GetAttribute("ROTATION");
}

double sind(double angle)
{
	double angelradians = angle * M_PI / 180.0;
	return sin(angelradians);
}
double cosd(double angle)
{
	double angelradians = angle * M_PI / 180.0;
	return cos(angelradians);
}

void BalloonController::Update(double dTime)
{
	double Fa = 0.0;  //����������
	double Fpod = 0.0; //���������
	// Get current attributes values
	TA_Point3D posit;
	m_pPosAttr->GetValue(posit);

	// Handle keyboard
	if (m_pInputManager->IsKeyPressed())
	{
		TA_Key key = m_pInputManager->GetKey();
		switch (key.code)
		{
		case TA_KEY_UP:
			koef += 0.1; break;
		case TA_KEY_DOWN:
			if (koef > 0)
			{
				koef -= 0.1;
			}
			break;
		case TA_KEY_ALT_L:
			if (windForce > 0)
			{
				windForce -= 1.0;
			}
			break;
		case TA_KEY_ALT_R:
			windForce += 1.0; break;
		case TA_KEY_RIGHT:
			angle += 5.0; break;	//���� ����� �����
		case TA_KEY_LEFT:			//�� ���� ��������� ���� ����� ����� ���������� �����������
			angle -= 5.0; break;
		};
	}
	Fa = ro_air * V * g * koef; //���� ��������. ��������� - ������ ������� (����������� ������������� ��� �����������).
	Fpod = Fa - massa * g; //���� �������� - ���� ������� = ��������� ����
	a = Fpod / massa;		//��������� ���������� � ����������.
	// Modify attribute values
	double dt = dTime - m_dOldTime;
	m_dOldTime = dTime;
	v += a * dt;
	if ((posit.z + v * dt + ((a * dt * dt) / 2)) > 0) {
		m_vel.x += windForce * cosd(angle) * dt;
		m_vel.y += windForce * sind(angle) * dt;
		m_vel.z += v * dt + ((a * dt * dt) / 2);
	}
	else
		m_vel.z = 0;

	// Save attribute values
	m_pPosAttr->SetValue(m_vel, dTime);

}
