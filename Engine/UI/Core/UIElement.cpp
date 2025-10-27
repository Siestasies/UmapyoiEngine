#include "UIElement.h"

namespace Uma_UI
{
	void UIElement::Update(float dt)
	{
		if (!enabled) return;

		for (auto& child : children)
		{
			if (child->enabled) child->Update();
		}
	}

	void UIElement::Render()
	{
		if (!visible) return;

		for (auto& child : children)
		{
			if (child->visible) child->Render();
		}
	}

	void UIElement::SetAnchor(Anchor preset)
	{
		switch (preset)
		{
		case Anchor::TopLeft:
			anchorMin = { 0.f, 1.f };
			anchorMax = { 0.f, 1.f };
			break;
		case Anchor::TopCentre:
			anchorMin = { 0.5f, 1.f };
			anchorMax = { 0.5f, 1.f };
			break;
		case Anchor::TopRight:
			anchorMin = { 1.f, 1.f };
			anchorMax = { 1.f, 1.f };
			break;
		case Anchor::MiddleLeft:
			anchorMin = { 0.f, 0.5f };
			anchorMax = { 0.f, 0.5f };
			break;
		case Anchor::MiddleCentre:
			anchorMin = { 0.5f, 0.5f };
			anchorMax = { 0.5f, 0.5f };
			break;
		case Anchor::MiddleRight:
			anchorMin = { 1.f, 0.5f };
			anchorMax = { 1.f, 0.5f };
			break;
		case Anchor::BottomLeft:
			anchorMin = { 0.f, 0.f };
			anchorMax = { 0.f, 0.f };
			break;
		case Anchor::BottomCentre:
			anchorMin = { 0.5f, 0.f };
			anchorMax = { 0.5f, 0.f };
			break;
		case Anchor::BottomRight:
			anchorMin = { 1.f, 0.f };
			anchorMax = { 1.f, 0.f };
			break;
		case Anchor::StretchAll:
			anchorMin = { 0.f, 0.f };
			anchorMax = { 1.f, 1.f };
			break;
		default:
			break;
		}
	}

	Rect UIElement::GetRect() const
	{
		if (!parent) return Rect(anchoredPosition.x, anchoredPosition.y, sizeDelta.x, sizeDelta.y);

		Rect parentRect = parent->GetRect();
		float anchorX = parentRect.x + parentRect.width * anchorMin.x;
		float anchorY = parentRect.y + parentRect.height * anchorMin.y;
		float anchorW = parentRect.width * (anchorMax.x - anchorMin.x);
		float anchorH = parentRect.height * (anchorMax.y - anchorMin.y);

		return Rect(
			anchorX + anchoredPosition.x - sizeDelta.x * pivot.x,
			anchorY + anchoredPosition.y - sizeDelta.y * pivot.y,
			anchorW + sizeDelta.x,
			anchorH + sizeDelta.y
		);
	}

	UIElement* FindByName(const std::string& name)
	{
		if (this->name == name) return this;

		for (const auto& child : children)
		{
			UIElement* result = child->FindByName(name);
			if (result != nullptr) return result;
		}

		return nullptr;
	}

	UIElement* FindByTag(const std::string& tag)
	{
		if (this->tag == tag) return this;

		for (const auto& child : children)
		{
			UIElement* result = child->FindByTag(tag);
			if (result != nullptr) return result;
		}

		return nullptr;
	}
}