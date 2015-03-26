#pragma once
#include "d2dsubview.h"
class SV_ProgressBar :
	public D2DSubView
{
public:
	SV_ProgressBar(D2DView* view, D2DSubView* parent);
	~SV_ProgressBar(void);

	/** Draws this sub-view */
	virtual void Draw(const D2D1_RECT_F& clientRectAbs, float deltaTime);
		
	/** Sets the progress of this bar */
	void SetProgress(float p);

protected:
	/** Progress in range 0..1 */
	float Progress;
};

