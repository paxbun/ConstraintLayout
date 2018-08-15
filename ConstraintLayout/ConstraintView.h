#pragma once
#include "App.h"

enum Direction
{
	DIRECTION_UNKNOWN,
	DIRECTION_LEFT,
	DIRECTION_RIGHT,
	DIRECTION_TOP,
	DIRECTION_BOTTOM
};

struct ViewElement
{
	float width, height;
	float r, g, b, a;
	std::wstring content;

	inline ViewElement()
		: width(0), height(0), r(0), g(0), b(0)
	{}

	inline ViewElement(
		float width, float height,
		float r, float g, float b, float a,
		std::wstring content = L""
	)
		: width(width), height(height), r(r), g(g), b(b), a(a), content(content)
	{}
};

struct ConstraintViewElement
{
	ViewElement elem;
	std::string name;
	float x, y;
	float width, height;
	struct Constraint
	{
		size_t target;
		Direction targetDirection;
		float value;
	} constraint[4];

	inline ConstraintViewElement()
	{}

	inline ConstraintViewElement(
		const ViewElement & elem,
		const std::string & name
	)
		: elem(elem), name(name)
	{}
};

class ConstraintView :
	public App
{
public:
	ConstraintView();
	virtual ~ConstraintView();

	virtual bool OnCreate() override;
	virtual void OnKeyDown(UINT8 key) override;
	virtual void OnKeyUp(UINT8 key) override;
	virtual void OnMouse(SHORT x, SHORT y, MouseCommands cmd) override;
	virtual bool OnSizeChange(WORD x, WORD y) override;
	virtual bool OnUpdate() override;
	virtual bool OnRender() override;
	virtual void OnDestroy() override;
	
private:

	HWND	_hWnd;
	int		_width;
	int		_height;
	ID2D1HwndRenderTarget	*	_pRT;
	ID2D1SolidColorBrush	*	_pBrush;
	ID2D1Effect				*	_pEffect;
	IDWriteTextFormat		*	_pFormat;
	
	DirectedGraph<ConstraintViewElement>	_elementDependencies;
	std::vector<size_t>						_topologicalSort;
	std::map<std::string, size_t>			_name_map;

	struct _AddElementArg
	{
		std::string name;
		Direction direction;
		float value;
	};

	bool _AddElement(
		const ViewElement & elem,
		std::string name,
		std::vector<_AddElementArg> dependency
		= std::vector<_AddElementArg>()
	);

	bool _InitializeWindow();
	bool _InitializeD2D();
	bool _InitializeElements();
	bool _UpdateDependency();
	void _DestroyD2D();

	static size_t const _idx_nan;
	static float const _nan;
};

