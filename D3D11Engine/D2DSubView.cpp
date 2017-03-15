#include "pch.h"
#include "D2DSubView.h"
#include "D2DView.h"

D2DSubView::D2DSubView(D2DView* view, D2DSubView* parent)
{
	//Layer = NULL;
	MainView = view;
	Parent = parent;
	ViewRect = D2D1::RectF(0,0,0,0);
	Level = 0;

	Hidden = false;

	if(parent)
		parent->AddChild(this);
	else
	{
		InitControls();
	}

	MainView->GetRenderTarget()->CreateLayer(NULL, &Layer);
}


D2DSubView::~D2DSubView(void)
{
	//if(Layer)Layer->Release();

	
	for(std::list<D2DSubView*>::iterator it = Children.begin();it != Children.end();it++)
	{
		delete (*it);
	}
}

/** Sets the level on this */
void D2DSubView::SetLevel(int level)
{
	Level = Level;
}

/** Aligns this control under the specified */
void D2DSubView::AlignUnder(D2DSubView* view, float space)
{
	SetPosition(D2D1::Point2F(view->GetRect().left, view->GetRect().bottom + space));
}

/** Aligns this control right to the specified */
void D2DSubView::AlignRightTo(D2DSubView* view, float space)
{
	SetPosition(D2D1::Point2F(view->GetRect().right + space, view->GetRect().top));
}

/** Aligns this control left to the specified */
void D2DSubView::AlignLeftTo(D2DSubView* view, float space)
{
	SetPosition(D2D1::Point2F(view->GetRect().left - space - GetSize().width, view->GetRect().top));
}

/** Sets the position and size of this sub-view */
void D2DSubView::SetRect(const D2D1_RECT_F& rect)
{
	ViewRect = rect;
}

/** Returns the rect of this control */
const D2D1_RECT_F& D2DSubView::GetRect()
{
	return ViewRect;
}

/** Draws this sub-view */
void D2DSubView::Draw(const D2D1_RECT_F& clientRectAbs, float deltaTime)
{
	//Set up the layer for this control
	MainView->GetRenderTarget()->SetTransform(D2D1::Matrix3x2F::Translation(clientRectAbs.left,clientRectAbs.top));

	// Debug
	//MainView->GetBrush()->SetColor(D2D1::ColorF(1,1,1,1));
	//MainView->GetRenderTarget()->DrawRectangle(ViewRect, MainView->GetBrush());

	D2D1_RECT_F r;
	r.top=0;
	r.left=0;
	r.right=clientRectAbs.right - clientRectAbs.left;
	r.bottom=clientRectAbs.bottom - clientRectAbs.top;

	MainView->GetRenderTarget()->PushAxisAlignedClip(r, D2D1_ANTIALIAS_MODE_ALIASED);

	D2D1_RECT_F rectAbs;
	rectAbs.left = clientRectAbs.left + ViewRect.left;
	rectAbs.top = clientRectAbs.top + ViewRect.top;
	rectAbs.right = clientRectAbs.left + ViewRect.right;
	rectAbs.bottom = clientRectAbs.top + ViewRect.bottom;

	// Draw children
	for(std::list<D2DSubView*>::iterator it = Children.begin();it != Children.end();it++)
	{
		//D2D1_RECT_F cr;
		//cr.left = clientRectAbs.left + (*it)->GetRect().left;
		//cr.top = clientRectAbs.top + (*it)->GetRect().top;
		//cr.right = clientRectAbs.left + (*it)->GetRect().right;
		//cr.bottom = clientRectAbs.top + (*it)->GetRect().bottom;

		//MainView->GetRenderTarget()->SetTransform(D2D1::Matrix3x2F::Translation(clientRectAbs.left,clientRectAbs.top));

		if(!(*it)->IsHidden())
			(*it)->Draw(rectAbs, deltaTime);
	}

	MainView->GetRenderTarget()->PopAxisAlignedClip();

	//Set up the layer for this control
	MainView->GetRenderTarget()->SetTransform(D2D1::Matrix3x2F::Translation(clientRectAbs.left,clientRectAbs.top));
}

/** Updates the subview */
void D2DSubView::Update(float deltaTime)
{
	if(Hidden)
		return;

	for(std::list<D2DSubView*>::iterator it = Children.begin();it != Children.end();it++)
	{
		(*it)->Update(deltaTime);
	}
}

/** Processes a window-message. Return false to stop the message from going to children */
bool D2DSubView::OnWindowMessage(HWND hWnd, unsigned int msg, WPARAM wParam, LPARAM lParam, const D2D1_RECT_F& clientRectAbs)
{
	// Process children
	for(std::list<D2DSubView*>::iterator it = Children.begin();it != Children.end();it++)
	{
		D2D1_RECT_F cr;
		cr.left = clientRectAbs.left + (*it)->GetRect().left;
		cr.top = clientRectAbs.top + (*it)->GetRect().top;
		cr.right = clientRectAbs.left + (*it)->GetRect().right;
		cr.bottom = clientRectAbs.top + (*it)->GetRect().bottom;

		if(!(*it)->IsHidden())
			if(!(*it)->OnWindowMessage(hWnd, msg, wParam, lParam, cr))
				return false;
	}

	return true;
}

/** Adds a child to this subview */
void D2DSubView::AddChild(D2DSubView* child)
{
	if(child->GetParent() != this)
	{
		LogWarn() << "Trying to add a D2DSubView-Child with an other parent than this!";
		return;
	}

	child->SetLevel(Level + 1);
	child->InitControls();
	Children.push_back(child);
}

/** Deletes a child from this subview */
void D2DSubView::DeleteChild(D2DSubView* child)
{
	child->SetHidden(true); // FIXME
	//Children.remove(child);
	//delete child;
}

/** Sets the position and size */
void D2DSubView::SetPositionAndSize(const D2D1_POINT_2F& position, const D2D1_SIZE_F& size)
{
	SetRect(D2D1::RectF(position.x, position.y, position.x + size.width, position.y + size.height));
}

/** Point inside rect? */
bool D2DSubView::PointInsideRect(const D2D1_POINT_2F& p, const D2D1_RECT_F& r)
{
	if( p.x > r.left && p.x < r.right &&
		p.y > r.top && p.y < r.bottom)
	{
		return true;
	}

	return false;
}

bool D2DSubView::PointInsideRect(const POINT& p, const D2D1_RECT_F& r)
{
	if( (float)p.x > r.left && (float)p.x < r.right &&
		(float)p.y > r.top && (float)p.y < r.bottom)
	{
		return true;
	}

	return false;
}

/** Sets the position of the control */
void D2DSubView::SetPosition(const D2D1_POINT_2F& position)
{
	D2D1_RECT_F r = ViewRect;

	r.right -= r.left;
	r.bottom -= r.top;
	r.left = position.x;
	r.top = position.y;

	r.right += r.left;
	r.bottom += r.top;

	SetRect(r);
}

/** Sets the size of this control */
void D2DSubView::SetSize(const D2D1_SIZE_F& size)
{
	D2D1_RECT_F r = ViewRect;
	r.right = r.left + size.width;
	r.bottom = r.top + size.height;

	SetRect(r);
}

/** Returns the size of this control */
D2D1_SIZE_F D2DSubView::GetSize()
{
	return D2D1::SizeF(ViewRect.right - ViewRect.left, ViewRect.bottom - ViewRect.top);
}

/** Returns the position of this control */
D2D1_POINT_2F D2DSubView::GetPosition()
{
	return D2D1::Point2F(ViewRect.left, ViewRect.top);
}

/** Sets if this control is hidden */
void D2DSubView::SetHidden(bool hidden)
{
	Hidden = hidden;
}

/** Returns if this control is hidden */
bool D2DSubView::IsHidden()
{
	return Hidden;
}

