#include <math.h>
#include "ballooncontroller.h"

#define M_PI 3.14159265358979323846	//числа подобраны для удобства демонстрации
#define g 9.8		//ускорение свободного падения
#define ro_air 1.2	//плотность воздуха
#define V 1000.0	//объем шара
#define massa 1200 //масса шара (к примеру, с корзиной, горелкой, людьми)

BalloonController::BalloonController() :
	m_pInputManager(TA_ResourceManager::GetInstance()->GetInputManager())
	, m_dOldTime(0)
	, m_dThrustForce(0)
	, m_vel(0)
	, v(0)				//скорость шара
	, a(0)				//ускорение шара
	, koef(1)			//интерпретация изменения температуры внутри шара
	, windForce(0)		//сила ветра
	, angle(0)			//направление ветра
{
	m_iType = MOVEMENT_CONTROLLER; //тип контроллера - для описания движения объекта
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
	double Fa = 0.0;  //Архимедова
	double Fpod = 0.0; //Подъемная
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
			angle += 5.0; break;	//угол атаки ветра
		case TA_KEY_LEFT:			//за счет изменения угла атаки ветра изменяется направление
			angle -= 5.0; break;
		};
	}
	Fa = ro_air * V * g * koef; //Сила Архимеда. Коэффиент - модель горелки (температура увеличивается или уменьшается).
	Fpod = Fa - massa * g; //Сила Архимеда - Сила Тяжести = Подъемная сила
	a = Fpod / massa;		//Ускорение получаемое в результате.
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
