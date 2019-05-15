
#ifndef __avatar_controller_h__
#define __avatar_controller_h__

#include "terranet_api.h"

class AvatarController : public TA_Controller
{
public:
	AvatarController();
	virtual ~AvatarController() {}

	virtual void Init(TA_Resource* pResource);
	virtual void Update(double dTime);
	TA_Camera* pCam;
protected:
	double m_dOldTime;
	TA_StateAttribute* m_pPosAttr;
	TA_StateAttribute* m_pRotAttr;
	TA_ResourceManager* m_pResourceManager;
	TA_InputManager* m_pInputManager;
	virtual TA_Controller* Clone() { return new AvatarController(); }
};
#endif
