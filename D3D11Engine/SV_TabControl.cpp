#include "pch.h"
#include "SV_TabControl.h"
#include "D2DView.h"
#include "SV_Panel.h"

const float SV_TABCONTROL_HEADER_SIZE_Y = 20.0f;

SV_TabControl::SV_TabControl(D2DView* view, D2DSubView* parent) : D2DSubView(view, parent)
{
	TabPanel = new SV_Panel(view, this);
	TabPanel->SetRect(D2D1::RectF(0, SV_TABCONTROL_HEADER_SIZE_Y, GetSize().width, GetSize().height));
	TabPanel->SetRenderMode(SV_Panel::PR_SolidColor);
	TabPanel->SetPanelColor(D2D1::ColorF(0.6f,0.6f,0.6f, 1.0f));
	TabPanel->SetDarkOverlay(true);
	TabPanel->SetGlossyOutline(true);

	TabSwitchedCallback = NULL;
	TabSwitchedUserdata = NULL;

	ActiveTab = NULL;

	OnlyShowActiveTab = false;
}


SV_TabControl::~SV_TabControl(void)
{
}

SV_TabControl_Tab::SV_TabControl_Tab()
{
	CaptionLayout = NULL;
}

SV_TabControl_Tab::~SV_TabControl_Tab()
{
	if(CaptionLayout)CaptionLayout->Release();
}

void SV_TabControl::SetTabCaption(const std::string& caption, SV_TabControl_Tab* tab)
{
	tab->Caption = caption;

	if(tab->CaptionLayout)tab->CaptionLayout->Release();
	MainView->GetWriteFactory()->CreateTextLayout(
		Toolbox::ToWideChar(caption).c_str(),      // The string to be laid out and formatted.
		caption.length(),  // The length of the string.
		MainView->GetDefaultTextFormat(),  // The text format to apply to the string (contains font information, etc).
		ViewRect.right - ViewRect.left,         // The width of the layout box.
		SV_TABCONTROL_HEADER_SIZE_Y,         // The height of the layout box.
		&tab->CaptionLayout  // The IDWriteTextLayout interface pointer.
		);

	if(tab->CaptionLayout)
	{
		tab->CaptionLayout->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_LEADING);
		tab->CaptionLayout->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_CENTER);
	}
	else
	{
		LogWarn() << "Failed to create TextLayout for caption '" << caption << "'";
	}
}

/** Draws this sub-view */
void SV_TabControl::Draw(const D2D1_RECT_F& clientRectAbs, float deltaTime)
{
	MainView->GetRenderTarget()->SetTransform(D2D1::Matrix3x2F::Translation(clientRectAbs.left,clientRectAbs.top));

	D2D1_RECT_F activeRect;
	float activeX = 0.0f;
	float x = 0.0f;
	for(std::map<std::string, SV_TabControl_Tab>::iterator it = Tabs.begin(); it != Tabs.end(); it++)
	{
		if(OnlyShowActiveTab && &(*it).second != ActiveTab)
					continue;

		float width = D2DView::GetLabelTextWidth((*it).second.CaptionLayout, (*it).first);

		D2D1_RECT_F tabRect = D2D1::RectF(x + ViewRect.left, ViewRect.top, x + ViewRect.left + width + 10.0f, ViewRect.top + SV_TABCONTROL_HEADER_SIZE_Y);

		

		//Outer shadow
		MainView->DrawSmoothShadow(&tabRect, 10, 0.5, true);

		if(&(*it).second == ActiveTab)
		{
			activeRect = tabRect;
			activeX = x;
		}else // Active tab draws its text later
		{
			MainView->GetBrush()->SetColor(D2D1::ColorF(0,0,0,0.5));
			MainView->GetRenderTarget()->DrawRectangle(tabRect, MainView->GetBrush());

			MainView->GetBrush()->SetColor(D2D1::ColorF(1,1,1,1));

			if((*it).second.CaptionLayout)
				MainView->GetRenderTarget()->DrawTextLayout(D2D1::Point2F(x + 5.0f + ViewRect.left, ViewRect.top), (*it).second.CaptionLayout, MainView->GetBrush());
		}

		x += width + 10.0f;
	}

	D2DSubView::Draw(clientRectAbs, deltaTime);

	if(ActiveTab)
	{
		D2D1_RECT_F sc = activeRect;

		//sc.top += 2;
		sc.bottom += 2;
		sc.left += 1;
		sc.right -= 1;

		MainView->GetBrush()->SetColor(TabPanel->GetPanelColor());
		MainView->GetRenderTarget()->FillRectangle(sc, MainView->GetBrush());

		sc.left -= 1;
		sc.right += 1;

		// Draw a border line
		MainView->GetLinearReflectBrush()->SetEndPoint(D2D1::Point2F(sc.left,sc.top));
		MainView->GetLinearReflectBrush()->SetStartPoint(D2D1::Point2F(sc.right,sc.bottom));

		MainView->GetRenderTarget()->DrawLine(D2D1::Point2F(sc.left, sc.top), D2D1::Point2F(sc.right, sc.top), MainView->GetLinearReflectBrush());
		MainView->GetRenderTarget()->DrawLine(D2D1::Point2F(sc.left, sc.top), D2D1::Point2F(sc.left, sc.bottom - 1), MainView->GetLinearReflectBrush());
		MainView->GetRenderTarget()->DrawLine(D2D1::Point2F(sc.right, sc.top), D2D1::Point2F(sc.right, sc.bottom - 1), MainView->GetLinearReflectBrush());

		D2DView::ShrinkRect(&sc, 1);

		MainView->GetBrush()->SetColor( SV_DEF_INNER_LINE_COLOR );
		MainView->GetRenderTarget()->DrawLine(D2D1::Point2F(sc.left, sc.top), D2D1::Point2F(sc.right, sc.top), MainView->GetBrush());
		MainView->GetRenderTarget()->DrawLine(D2D1::Point2F(sc.left, sc.top), D2D1::Point2F(sc.left, sc.bottom+1), MainView->GetBrush());
		MainView->GetRenderTarget()->DrawLine(D2D1::Point2F(sc.right, sc.top), D2D1::Point2F(sc.right, sc.bottom+1), MainView->GetBrush());

		// Emboss it
		MainView->GetBrush()->SetColor(D2D1::ColorF(0,0,0,0.7f));
		
		if(ActiveTab->CaptionLayout)
			MainView->GetRenderTarget()->DrawTextLayout(D2D1::Point2F(1.0f + activeX + 5.0f + ViewRect.left, 1.0f + ViewRect.top), ActiveTab->CaptionLayout, MainView->GetBrush());

		// Draw caption
		MainView->GetBrush()->SetColor(D2D1::ColorF(1,1,1,1));

		if(ActiveTab->CaptionLayout)
			MainView->GetRenderTarget()->DrawTextLayout(D2D1::Point2F(activeX + 5.0f + ViewRect.left, ViewRect.top), ActiveTab->CaptionLayout, MainView->GetBrush());
	}
}

/** Processes a window-message. Return false to stop the message from going to children */
bool SV_TabControl::OnWindowMessage(HWND hWnd, unsigned int msg, WPARAM wParam, LPARAM lParam, const D2D1_RECT_F& clientRectAbs)
{
	switch(msg)
	{
	case WM_LBUTTONDOWN:
		{
			POINT p = D2DView::GetCursorPosition();

			float x = 0.0f;
			for(std::map<std::string, SV_TabControl_Tab>::iterator it = Tabs.begin(); it != Tabs.end(); it++)
			{
				if(OnlyShowActiveTab && &(*it).second != ActiveTab)
					continue;

				float width = D2DView::GetLabelTextWidth((*it).second.CaptionLayout, (*it).first);

				D2D1_RECT_F tabRect = D2D1::RectF(x + ViewRect.left, ViewRect.top, x + ViewRect.left + width + 10.0f, ViewRect.top + SV_TABCONTROL_HEADER_SIZE_Y);
				if(PointInsideRect(D2D1::Point2F((float)p.x, (float)p.y), tabRect))
				{
					SetActiveTab((*it).second.Caption);
					return false;
				}

				x += width + 10.0f;
			}
		}
		break;
	}
	return D2DSubView::OnWindowMessage(hWnd, msg, wParam, lParam, clientRectAbs);
}

/** Shows/Hides all controls in a SV_TabControl_Tab */
void SV_TabControl::SetTabVisibility(SV_TabControl_Tab* tab, bool hide)
{
	for(std::list<D2DSubView*>::iterator it = tab->Controls.begin(); it != tab->Controls.end(); it++)
	{
		(*it)->SetHidden(hide);
	}
}

/** Adds a control to a tab */
void SV_TabControl::AddControlToTab(D2DSubView* control, const std::string& tab)
{
	Tabs[tab].Controls.push_back(control);
	
	control->SetHidden(true);

	if(!Tabs[tab].Caption.length())
		SetTabCaption(tab, &Tabs[tab]);

	if(!ActiveTab)
		ActiveTab = &Tabs[tab];

	SetActiveTab(ActiveTab->Caption);
}

/** Returns the tab-panel of this TabControl. All subviews must be added to this panel. */
SV_Panel* SV_TabControl::GetTabPanel()
{
	return TabPanel;
}

/** Sets the position and size of this sub-view */
void SV_TabControl::SetRect(const D2D1_RECT_F& rect)
{
	D2DSubView::SetRect(rect);

	// Update tab-panel size
	TabPanel->SetRect(D2D1::RectF(0, SV_TABCONTROL_HEADER_SIZE_Y, GetSize().width, GetSize().height));
}

/** Sets active tab */
void SV_TabControl::SetActiveTab(const std::string& tab)
{
	if(Tabs.find(tab) == Tabs.end())
		return; // Don't have that

	// Hide all
	for(std::map<std::string, SV_TabControl_Tab>::iterator it = Tabs.begin(); it != Tabs.end(); it++)
	{
		SetTabVisibility(&(*it).second, true);
	}

	// Set and Show active
	ActiveTab = &Tabs[tab];
	SetTabVisibility(ActiveTab, false);
	if(TabSwitchedCallback)
		TabSwitchedCallback(this, TabSwitchedUserdata);
}

/** Sets the callback */
void SV_TabControl::SetTabSwitchedCallback(SV_TabSwitchedCallback fn, void* userdata)
{
	TabSwitchedCallback = fn;
	TabSwitchedUserdata = userdata;
}

/** Returns the name of the active tab */
std::string SV_TabControl::GetActiveTab()
{
	return ActiveTab->Caption;
}

/** If true, this will only show the currently active tab */
void SV_TabControl::SetOnlyShowActiveTab(bool value)
{
	OnlyShowActiveTab = value;
}