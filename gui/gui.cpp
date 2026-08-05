/*
 * ceeGUI
 * Copyright (C) 2025 2026 Chloe Eather
 *
 * This program is free software: you can redistribute it and/or modify it under the
 * terms of the GNU General Public License as published by the Free Software
 * Foundation, either version 3 of the License, or (at your option) any later
 * version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. See the GNU General Public License for more
 * details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program. If not, see <https://www.gnu.org/licenses/>.
 */

#include <cee/gui/gui.h>
#include <object_impl.h>

#include <cee/profiler/profiler.h>

#include <memory>

namespace cee {
namespace gui {
	class RootNode : public Object {
	public:
		RootNode(std::shared_ptr<Context> ctx) : m_Child(nullptr) { m_Impl->ctx = ctx; }

		inline void SetChild(Object *child) {
			if (m_Child != nullptr) {
				RemoveChild(m_Child);
			}
			m_Child = child;
			AddChild(child);
		}

		inline Object *GetChild() {
			return m_Child;
		}

		const inline Object *GetChild() const {
			return m_Child;
		}

		void SetClip(const Size &clip) { m_Clip = { 0.f, 0.f, clip.x, clip.y }; }

		void StartMeasure(float x, float y, float w, float h) {
			m_Impl->Measure({ x, y, w, h});
		}

		void StartArrange(const Rect &viewportRect) {
			m_Impl->Arrange(viewportRect, viewportRect);
		}

		void StartRender() {
			m_Impl->RenderTree();
		}

	protected:
		virtual bool HasClip() const override { return true; }
		virtual Size Transform() const override { return { 0.f, 0.f }; }
		virtual bool HasTransform() const override { return true; }
		// TODO get actual screen size
		virtual Rect Clip() const override { return m_Clip; }

		virtual Size OnMeasure(const Constraints &c) override {
			if (m_Child == nullptr)
				return { 0.f, 0.f };
			return GetImpl(m_Child)->Measure(c);
		}

		virtual void OnArrange() override {
			if (m_Child == nullptr)
				return;
			GetImpl(m_Child)->Arrange(m_Impl->m_Rect, m_Impl->m_AbsoluteRect);
		}

		virtual bool CanHaveChildren() const override { return true; }

	private:
		Object *m_Child;
		Rect m_Clip;
	};

	static RootNode *g_Root = nullptr;
	static std::shared_ptr<Context> g_Ctx;

	namespace internal {
		int PrepareNode(void *ptr) {
			Object::Impl& impl = *reinterpret_cast<Object::Impl *>(ptr);
			impl.ctx = g_Ctx;
			return 0;
		}
	}

	int Init(Logger logger) {
		g_Ctx = std::make_shared<Context>(logger);
		g_Root = new RootNode(g_Ctx);
		if (g_Root == nullptr)
			return -1;
		g_Root->SetDebugName("rootNode");
		return 0;
	}

	void Shutdown() {
		delete g_Root;
		g_Root = nullptr;
		g_Ctx.reset();
	}

	void SetRootNode(Widget *node) {
		if (node == nullptr)
			return;
		g_Root->SetChild(node);
	}

	int BeginFrame(const Size &viewport) {
		g_Root->SetClip(viewport);
		g_Ctx->SetViewport(viewport);
		return 0;
	}

	int Render(const Size &viewport) {
		PROFILE_SCOPE("GUI frame");
		Rect viewportRect = { 0.f, 0.f, viewport.w, viewport.h };
		{
			PROFILE_SCOPE("GUI Measure");
			g_Root->StartMeasure(0.f, 0.f, viewport.w, viewport.h);
		}
		{
			PROFILE_SCOPE("GUI Arrange");
			g_Root->StartArrange(viewportRect);
		}
		{
			PROFILE_SCOPE("GUI render context prepare");
			g_Ctx->SetUniform(Context::GuiShader::Flat, "uProj", g_Ctx->GetProjection());
			g_Ctx->SetUniform(Context::GuiShader::Texture, "uProj", g_Ctx->GetProjection());
		}
		{
			PROFILE_SCOPE("GUI Draw");
			g_Root->StartRender();
		}
		g_Ctx->Flush();
		return 0;
	}

	void EndFrame() {
	}
}
}

