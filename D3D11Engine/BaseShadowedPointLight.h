#pragma once

struct BaseVobInfo;
class BaseShadowedPointLight
{
public:
	BaseShadowedPointLight(void);
	virtual ~BaseShadowedPointLight(void);

	/** Called when a vob got removed from the world */
	virtual void OnVobRemovedFromWorld(BaseVobInfo* vob){};
};

