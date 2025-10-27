#pragma once

#include "../Core/UITypes.h"

#include <vector>
#include <memory>
#include <string>

namespace Uma_UI
{
	class UIElement
	{
	public:
		virtual ~UIElement() = default;

		virtual void Update(float dt);
		virtual void Render();

		void SetAnchor(Anchor preset);
		Rect GetRect() const;

		template <typename T>
		T* AddChild(std::unique_ptr<T> child)
		{
			child->parent = this;
			T* ptr = child.get();
			children.push_back(std::move(child));
			return ptr;
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

		template <typename T>
		T* FindByNameAs(const std::string& name)
		{
			return dynamic_cast<T*>(FindByName(name));
		}

	public:
		UIElement* parent = nullptr;
		std::vector<std::unique_ptr<UIElement>> children;

		Vec2 anchorMin{};
		Vec2 anchorMax{};
		Vec2 anchoredPosition{};
		Vec2 sizeDelta{};
		Vec2 pivot{};

		bool enabled = true;
		bool visible = true;
		std::string name;
		std::string tag;

		Rect rect{};
		bool layoutDirty = true;
	};
}