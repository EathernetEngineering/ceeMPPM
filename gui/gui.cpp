/*
 * CeeHealth
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
#include <context.h>
#include <log.h>

#include <cee/profiler/profiler.h>

#include <memory>

namespace cee {
namespace gui {
	class RootNode : public Object {
	public:
		RootNode(Context *ctx) : m_Child(nullptr) { this->ctx = ctx; }

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

		void SetClip(const glm::vec2 &clip) { m_Clip = { 0.f, 0.f, clip.x, clip.y }; }

	protected:
		virtual bool HasClip() const override { return true; }
		virtual glm::mat4 Transform() const override { return glm::identity<glm::mat4>(); }
		virtual bool HasTransform() const override { return true; }
		// TODO get actual screen size
		virtual Rect Clip() const override { return m_Clip; }

		virtual Size OnMeasure(const Constraints &c) override {
			if (m_Child == nullptr)
				return { 0.f, 0.f };
			return m_Child->_Measure(c);
		}

		virtual void OnArrange() override {
			if (m_Child == nullptr)
				return;
			m_Child->_Arrange(m_Rect, m_AbsoluteRect);
		}

	private:
		Object *m_Child;
		Rect m_Clip;
	};

	static RootNode *g_Root = nullptr;
	static std::unique_ptr<Context> g_Ctx;

	int Init() {
		g_Ctx = std::make_unique<Context>();
		g_Root = new RootNode(g_Ctx.get());
		if (g_Root == nullptr)
			return -1;
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

	int BeginFrame(const glm::vec2 &viewport) {
		g_Root->SetClip(viewport);
		g_Ctx->SetViewport(viewport);
		return 0;
	}

	int Render(float viewportWidth, float viewportHeight) {
		PROFILE_SCOPE("GUI frame");
		Rect viewportRect = { 0.f, 0.f, viewportWidth, viewportHeight };
		{
			PROFILE_SCOPE("GUI Measure");
			g_Root->_Measure({ 0.f, 0.f, viewportWidth, viewportHeight });
		}
		{
			PROFILE_SCOPE("GUI Arrange");
			g_Root->_Arrange(viewportRect, viewportRect);
		}
		{
			PROFILE_SCOPE("GUI render context prepare");
			g_Ctx->UseShader(Context::Shader::Flat);
			g_Ctx->SetUniform("uProj", g_Ctx->GetProjection());
		}
		{
			PROFILE_SCOPE("GUI Draw");
			g_Root->_RenderTree();
		}
		g_Ctx->Flush();
		return 0;
	}

	void EndFrame() {
	}

	void InitLogger() {
		Log::Init();
	}

	void ShutdownLogger() {
		Log::Shutdown();
	}

	std::shared_ptr<spdlog::logger> GetLogger() {
		return Log::GetLogger();
	}
}
}

