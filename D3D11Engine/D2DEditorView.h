#pragma once
#include "D2DSubView.h"
#include "SV_Button.h"
#include "SV_TabControl.h"

class SV_Panel;
class SV_Checkbox;
class SV_Label;
class SV_Slider;
class SV_NamedSlider;
class D2DVobSettingsDialog;
class GVegetationBox;
struct MeshInfo;
struct WorldMeshInfo;
struct VobInfo;
struct SkeletalVobInfo;
class zCMaterial;
class zCVob;
class WidgetContainer;
struct SelectionInfo
{
	SelectionInfo()
	{
		Reset();
	}

	void Reset()
	{
		SelectedMesh = NULL;
		SelectedMaterial = NULL;
		SelectedVegetationBox = NULL;
		SelectedVobInfo = NULL;
		SelectedSkeletalVob = NULL;
	}

	MeshInfo* SelectedMesh;
	zCMaterial* SelectedMaterial;
	VobInfo* SelectedVobInfo;
	SkeletalVobInfo* SelectedSkeletalVob;
	GVegetationBox* SelectedVegetationBox;
};

class D2DEditorView :
	public D2DSubView
{
public:
	D2DEditorView(D2DView* view, D2DSubView* parent);
	virtual ~D2DEditorView(void);

	enum EditorMode
	{
		EM_IDLE,
		EM_PLACE_VEGETATION,
		EM_SELECT_POLY,
		EM_REMOVE_VEGETATION
	};

	/** Initializes the controls of this view */
	virtual XRESULT InitControls();

	/** Draws this sub-view */
	virtual void Draw(const D2D1_RECT_F& clientRectAbs, float deltaTime);

	/** Updates the subview */
	void Update(float deltaTime);

	/** Processes a window-message. Return false to stop the message from going to children */
	virtual bool OnWindowMessage(HWND hWnd, unsigned int msg, WPARAM wParam, LPARAM lParam, const D2D1_RECT_F& clientRectAbs);

	/** Finds the GVegetationBox from it's mesh-info */
	GVegetationBox* FindVegetationFromMeshInfo(MeshInfo* info);

	/** Called when a vob was removed from the world */
	XRESULT OnVobRemovedFromWorld(zCVob* vob);

protected:
	/** Visualizes a mesh info */
	void VisualizeMeshInfo(MeshInfo* m, const D3DXVECTOR4& color = D3DXVECTOR4(1,1,1,1), bool showBounds = false, const D3DXMATRIX* world = NULL);

	/** Sets the editor-mode */
	void SetEditorMode(EditorMode mode);

	/** Handles vegetationbox placement */
	void DoVegetationPlacement();

	/** Handles selection */
	void DoSelection();

	/** Handles vegetation removing */
	void DoVegetationRemove();

	/** Smoothes a mesh */
	void SmoothMesh(WorldMeshInfo* mesh, bool tesselate = false);

	/** Called on xBUTTONDOWN */
	void OnMouseButtonDown(int button);
	void OnMouseButtonUp(int button);
	void OnMouseClick(int button);
	void OnMouseWheel(int delta);

	/** Called on VK_DELETE */
	void OnDelete();

	/** Handles the editor movement */
	void DoEditorMovement();
	void ResetEditorCamera();

	/** Updates the selection panel */
	void UpdateSelectionPanel();

	/** Returns if the mouse is inside the editor window */
	bool IsMouseInsideEditorWindow();

	/** Places the currently dragged vegetation box */
	GVegetationBox* PlaceDraggedVegetationBox();

	/** Traces the set of placed vegatation boxes */
	GVegetationBox* TraceVegetationBoxes(const D3DXVECTOR3& wPos, const D3DXVECTOR3& wDir);


	/** Callbacks */
	/** Button to add a vegetation-volume was pressed */
	static void AddVegButtonPressed(SV_Button* sender, void* userdata);

	/** Button to add a vegetation-volume was pressed */
	static void FillVegButtonPressed(SV_Button* sender, void* userdata);

	/** Tab in main tab-control was switched */
	static void MainTabSwitched(SV_TabControl* sender, void* userdata);

	/** Tab in main tab-control was switched */
	static void RemoveVegButtonPressed(SV_Button* sender, void* userdata);

	/** Save/Load-Buttons */
	static void SaveLevelPressed(SV_Button* sender, void* userdata);
	static void LoadLevelPressed(SV_Button* sender, void* userdata);
	static void InfoPressed(SV_Button* sender, void* userdata);

	/** Tab in main tab-control was switched */
	static void VegetationAmountSliderChanged(SV_Slider* sender, void* userdata);

	/** Tab in main tab-control was switched */
	static void VegetationScaleSliderChanged(SV_Slider* sender, void* userdata);
	static void TextureSettingsSliderChanged(SV_Slider* sender, void* userdata);

	
	/** Editor enabled? */
	bool IsEnabled;

	/** Main editor panel */
	SV_Panel* MainPanel;
	SV_TabControl* MainTabControl;
	SV_TabControl* SelectionTabControl;

	/** Selection panel */
	SV_Panel* SelectedImagePanel;
	SV_Label* SelectedImageNameLabel;

	SV_NamedSlider* SelectedTexNrmStrSlider;
	SV_NamedSlider* SelectedTexSpecIntensSlider;
	SV_NamedSlider* SelectedTexSpecPowerSlider;
	SV_NamedSlider* SelectedTexDisplacementSlider;

	SV_NamedSlider* SelectedMeshTessAmountSlider;
	SV_NamedSlider* SelectedMeshRoundnessSlider;

	SV_Slider* SelectedVegSizeSlider;
	SV_Slider* SelectedVegAmountSlider;
	SV_Label* SelectedVegModifiedWarningLabel;

	D2DVobSettingsDialog* VobSettingsDialog;

	/** Current mode */
	EditorMode Mode;

	std::string TracedTexture;

	/** Vegetation-box specific values, valid in EM_PLACE_VEGETATION*/
	D3DXVECTOR3 DraggedBoxMinLocal;
	D3DXVECTOR3 DraggedBoxMaxLocal;
	D3DXVECTOR3 DraggedBoxCenter;
	
	SV_Checkbox* VegRestrictByTextureCheckBox;
	SV_Checkbox* VegCircularShapeCheckBox;

	/** Selection specific values */
	SV_Checkbox* SelectTrianglesOnlyCheckBox;
	bool SelectedSomething;
	D3DXVECTOR3 SelectedTriangle[3];
	D3DXVECTOR3 TracedPosition;
	MeshInfo* TracedMesh;
	GVegetationBox* TracedVegetationBox;
	zCMaterial* TracedMaterial;
	VobInfo* TracedVobInfo;
	SkeletalVobInfo* TracedSkeletalVobInfo;
	SelectionInfo Selection;

	/** Mouse controls */
	bool MButtons[3];
	DWORD ButtonPressedLastTime[3];
	bool MMovedAfterClick;
	HCURSOR MPrevCursor;
	float MMWDelta; // Mousewheel delta since last selection

	/** Editor controls */
	POINT CStartMousePosition;
	float CPitch;
	float CYaw;
	D3DXMATRIX CStartWorld;
	bool Keys[256];

	/** Vegetation settings */
	float VegLastUniformScale;

	WidgetContainer* Widgets;
};

