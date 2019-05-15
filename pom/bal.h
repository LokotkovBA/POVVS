
#ifndef __aircraft_controller_h__
#define __aircraft_controller_h__

#include "terranet_api.h"

class BalloonController : public TA_Controller
{
public:
	BalloonController();
	virtual ~BalloonController() {}

	virtual void Init(TA_Resource* pResource);
	virtual void Update(double dTime);
protected:
	double m_dOldTime;
	double m_dThrustForce;
	TA_Point3D m_vel;
	double v;
	double a;
	double koef;
	double windForce;
	double angle;

	TA_StateAttribute* m_pPosAttr;
	TA_StateAttribute* m_pRotAttr;
	TA_ResourceManager* m_pResourceManager;
	TA_InputManager* m_pInputManager;

	virtual TA_Controller* Clone() { return new BalloonController(); }
};


#endif
