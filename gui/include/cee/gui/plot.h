/*
 * ceeGUI
 * Copyright (C) 2026 Chloe Eather
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

#ifndef CEE_GUI_LINE_GRAPH_H_
#define CEE_GUI_LINE_GRAPH_H_

#include <cee/gui/widget.h>

namespace cee {
namespace gui {
	class Plot : public Widget {
	protected:
		Plot();
		Plot(const float *data, std::size_t count);
		Plot(const Color &color);
		Plot(const float *data, std::size_t count, const Color &color);

	public:

		void SetColor(const Color &color) { m_Color = color; }
		void SetData(const float *data, std::size_t count, std::size_t offset = 0);
		void ClearData() { m_Data.clear(); m_Points.clear(); }
		void ResizeData(std::size_t size) { m_Data.resize(size); m_DataStale = true; }

		void SetLineBreakPos(size_t pos) { m_LineBreakPos = pos; }
		void SetLineBreakWidth(size_t width) { m_LineBreakWidth = width; }

	protected:
		virtual bool HasClip() const override { return true; }
		virtual Rect Clip() const override;

		virtual Size OnMeasure(const Constraints &c) override;
		virtual void OnRender() override;

		virtual bool CanHaveChildren() const override { return false; }

	private:
		static std::vector<Point> &PlotPoints(Plot *o,
				const std::vector<float> &data,
				std::vector<Point> &points, size_t lineBreak = 0);

	private:
		size_t m_LineBreakPos = 0;
		size_t m_LineBreakWidth = 4;
		std::vector<float> m_Data;
		std::vector<Point> m_Points;
		Color m_Color;
		bool m_DataStale = true;

	public:
		template<typename T, typename ...Args>
		requires std::derived_from<T, Object>
		friend std::unique_ptr<T> CreateNode(Args &&...args);
	};
}
}

#endif

