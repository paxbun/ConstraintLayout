#include "stdafx.h"
#include "ConstraintView.h"

size_t const ConstraintView::_idx_nan	= std::numeric_limits<size_t>::max();
float const ConstraintView::_nan		= std::numeric_limits<float>::quiet_NaN();

ConstraintView::ConstraintView()
	: _elementDependencies(), _width(_desc->scrnWidth), _height(_desc->scrnHeight),
	_name_map({
		{ "ScreenLeft", _idx_nan - 1 },
		{ "ScreenRight", _idx_nan - 2 },
		{ "ScreenTop", _idx_nan - 3 },
		{ "ScreenBottom", _idx_nan - 4 }
	})
{}


ConstraintView::~ConstraintView()
{}

bool ConstraintView::OnCreate()
{
	if (!_InitializeWindow())
		return false;
	if (!_InitializeD2D())
		return false;

	return true;
}

void ConstraintView::OnKeyDown(UINT8 key)
{
}

void ConstraintView::OnKeyUp(UINT8 key)
{
}

void ConstraintView::OnMouse(SHORT x, SHORT y, MouseCommands cmd)
{
}

bool ConstraintView::OnSizeChange(WORD x, WORD y)
{
	_width = x;
	_height = y;
	if (_pRT)
		return SUCCEEDED(_pRT->Resize(D2D1::SizeU(x, y)));
	else
		return true;
}

bool ConstraintView::OnUpdate()
{
	_InitializeElements();

	static auto const valueByConstraint = [this](const ConstraintViewElement::Constraint & constraint) -> float
	{

		// If the constraint targets to the edge of the screen
		if (constraint.target == _idx_nan - 1)
			return 0;
		else if (constraint.target == _idx_nan - 2)
			return (float)_width;
		else if (constraint.target == _idx_nan - 3)
			return 0;
		else if (constraint.target == _idx_nan - 4)
			return (float)_height;


		if (constraint.target >= _elementDependencies.VerticesSize())
			return _nan;

		auto & target = _elementDependencies.VertexAt(constraint.target).value;
		switch (constraint.targetDirection)
		{
		case DIRECTION_LEFT:
			return target.x;
		case DIRECTION_RIGHT:
			return target.x + target.width;
		case DIRECTION_TOP:
			return target.y;
		case DIRECTION_BOTTOM:
			return target.y + target.height;
		}
		return _nan;
	};

	for (auto i : _topologicalSort)
	{
		auto & elem = _elementDependencies.VertexAt(i).value;
		auto left	= valueByConstraint(elem.constraint[0]) + elem.constraint[0].value;
		auto right	= valueByConstraint(elem.constraint[1]) - elem.constraint[1].value;
		auto top	= valueByConstraint(elem.constraint[2]) + elem.constraint[2].value;
		auto bottom	= valueByConstraint(elem.constraint[3]) - elem.constraint[3].value;

		float x, y, width, height;

		// horizontal
		if (isnan(left))
		{
			if (isnan(right))
				x = 0, width = 0;
			else
				x = right - elem.elem.width, width = elem.elem.width;
		}
		else
		{
			if (isnan(right))
				x = left, width = elem.elem.width;
			else if (elem.elem.width != 0)
				x = (_width - elem.elem.width) / 2.0f, width = elem.elem.width;
			else
				x = left, width = right - left;
		}

		// vertical
		if (isnan(top))
		{
			if (isnan(bottom))
				y = 0, height = 0;
			else
				y = bottom - elem.elem.height, height = elem.elem.height;
		}
		else
		{
			if (isnan(bottom))
				y = top, height = elem.elem.height;
			else if (elem.elem.height != 0)
				y = (_height - elem.elem.height) / 2.0f, height = elem.elem.height;
			else
				y = top, height = bottom - top;
		}

		elem.x = x, elem.y = y;
		elem.width = width, elem.height = height;
	}
	return true;
}

bool ConstraintView::OnRender()
{
	_pRT->BeginDraw();
	_pRT->Clear(D2D1::ColorF(0.0f, 0.2f, 0.4f));

	for (auto & elem : _elementDependencies)
	{
		auto & value = elem.value;
		_pBrush->SetColor(
			D2D1::ColorF(
				value.elem.r,
				value.elem.g,
				value.elem.b,
				value.elem.a
			)
		);
		auto rect = D2D1::RectF(
			value.x,
			value.y,
			value.x + value.width,
			value.y + value.height
		);
		_pRT->FillRectangle(rect, _pBrush);

		_pBrush->SetColor(
			D2D1::ColorF(1.0f, 1.0f, 1.0f, value.elem.a)
		);
		_pRT->DrawTextW(
			value.elem.content.c_str(), (UINT32)value.elem.content.size(),
			_pFormat, rect, _pBrush
		);
	}

	return SUCCEEDED(_pRT->EndDraw());
}

void ConstraintView::OnDestroy()
{
	_DestroyD2D();
}

bool ConstraintView::_AddElement(
	const ViewElement & elem,
	std::string name,
	std::vector<_AddElementArg> dependency)
{

	if (auto it = _name_map.find(name); it != _name_map.end())
		return false;


	ConstraintViewElement new_elem(elem, name);

	// Find the indices of targets
	for (int i = 0; i < 4 && (size_t)i < dependency.size(); i++)
	{
		if (dependency.at(i).name.empty())
		{
			new_elem.constraint[i].target = _idx_nan;
			new_elem.constraint[i].targetDirection = DIRECTION_UNKNOWN;
			new_elem.constraint[i].value = 0;
		}
		else if (auto it = _name_map.find(dependency.at(i).name); it != _name_map.end())
		{
			new_elem.constraint[i].target = it->second;
			new_elem.constraint[i].targetDirection = dependency.at(i).direction;
			new_elem.constraint[i].value = dependency.at(i).value;
		}
		else
			return false;
	}


	for (int i = (int)dependency.size(); i < 4; i++)
	{
		new_elem.constraint[i].target = _idx_nan;
		new_elem.constraint[i].targetDirection = DIRECTION_UNKNOWN;
		new_elem.constraint[i].value = 0;
	}

	auto new_idx = _elementDependencies.VerticesSize();
	_name_map.insert(std::make_pair(name, _elementDependencies.VerticesSize()));
	_elementDependencies.PushVertex(std::move(new_elem));

	auto & inserted = _elementDependencies.VertexAt(new_idx).value;

	for (int i = 0; i < 4; i++)
		if (inserted.constraint->target < _idx_nan - 4)
			_elementDependencies.PushEdge({ inserted.constraint->target, new_idx });

	return true;
}

bool ConstraintView::_InitializeWindow()
{
	RECT rect;
	rect.left = rect.top = 0;
	rect.right = _width; rect.bottom = _height;

	AdjustWindowRect(&rect, WS_OVERLAPPEDWINDOW, FALSE);

	_hWnd = CreateWindowEx(
		NULL,
		_className, _windowName, WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, CW_USEDEFAULT, rect.right - rect.left, rect.bottom - rect.top,
		NULL, NULL, _desc->hInstance, this
	);
	ShowWindow(_hWnd, _desc->nCmdShow);
	return _hWnd != NULL;
}

bool ConstraintView::_InitializeD2D()
{
#define CHECK(exp)				{\
	OutputDebugStringA(#exp);	\
	OutputDebugStringA("");		\
	if(FAILED(exp))				\
		return false;			}

	CHECK(_pD2DFactory->CreateHwndRenderTarget(
		D2D1::RenderTargetProperties(),
		D2D1::HwndRenderTargetProperties(
			_hWnd,
			D2D1::SizeU(
			(UINT32)(_width * _dpiX / 96.0f),
				(UINT32)(_height * _dpiY / 96.0f)
			)
		),
		&_pRT
	));

	CHECK(_pRT->CreateSolidColorBrush(
		D2D1::ColorF(1.0f, 1.0f, 1.0f),
		&_pBrush
	));



	CHECK(_pDWriteFactory->CreateTextFormat(
		L"Segoe UI Light",
		NULL,
		DWRITE_FONT_WEIGHT_NORMAL,
		DWRITE_FONT_STYLE_NORMAL,
		DWRITE_FONT_STRETCH_NORMAL,
		16,
		L"",	//Locale
		&_pFormat
	));

	return true;

#undef CHECK
}

bool ConstraintView::_InitializeElements()
{
	_topologicalSort.clear();
	_elementDependencies.Clear();
	_name_map = {
		{ "ScreenLeft", _idx_nan - 1 },
		{ "ScreenRight", _idx_nan - 2 },
		{ "ScreenTop", _idx_nan - 3 },
		{ "ScreenBottom", _idx_nan - 4 }
	};

	try
	{

		std::ifstream ifs("layout.json", std::ifstream::in);
		json o; ifs >> o;
		ifs.close();

		for (auto it = o.begin(); it != o.end(); it++)
		{
			auto & value = it.value();
			auto & content = value.at("content");
			auto & constraints = value.at("constraints");

			std::vector<_AddElementArg> args(4);

			for (auto jt = constraints.begin(); jt != constraints.end(); jt++)
			{
				static std::map<std::string, size_t> const dir2arg {
					{ "left", 0 },
					{ "right", 1 },
					{ "top", 2 },
					{ "bottom", 3 }
				};
				static std::map<std::string, Direction> const dir2dir{
					{ "left", DIRECTION_LEFT },
					{ "right", DIRECTION_RIGHT },
					{ "top", DIRECTION_TOP },
					{ "bottom", DIRECTION_BOTTOM }
				};

				auto & j_value = jt.value();

				size_t targ = dir2arg.at(jt.key());
				args[targ].name = j_value.at("name").get<std::string>();
				args[targ].direction = dir2dir.at(j_value.at("direction"));
				args[targ].value = j_value.at("value");
			}

			float width = content.at("width");
			float height = content.at("height");
			float r = 0.0f;
			float g = 0.0f;
			float b = 0.0f;
			float a = 1.0f;
			try
			{
				auto & color = content.at("color");
				r = color.at(0);
				g = color.at(1);
				b = color.at(2);
				a = color.at(3);
			}
			catch (const std::exception &)
			{}
			std::wstring text = ToWideChar(content.at("text").get<std::string>());


			if (!_AddElement(
				ViewElement(
					width, height,
					r, g, b, a,
					text
				),
				it.key(),
				args
			))
				return false;
		}
	}
	catch (const std::exception &)
	{
		return false;
	}

	return _UpdateDependency();
}

bool ConstraintView::_UpdateDependency()
{
	// Check loop
	auto scc = _elementDependencies.StronglyConnected();
	for (auto & i : scc)
		if (i.value.size() > 1)
			return false;

	// topological sort
	std::vector<size_t> tmp = _elementDependencies.Search();
	_topologicalSort = std::vector<size_t>(tmp.rbegin(), tmp.rend());

	return true;
}

void ConstraintView::_DestroyD2D()
{
	SafeRelease(&_pFormat);
	SafeRelease(&_pBrush);
	SafeRelease(&_pRT);
}

