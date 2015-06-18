#pragma once

struct BaseVobInfo;
class ShadowedPointLight
{
public:
	ShadowedPointLight(void);
	virtual ~ShadowedPointLight(void);

	/** Called when a vob got removed from the world */
	virtual void OnVobRemovedFromWorld(BaseVobInfo* vob){};
};

