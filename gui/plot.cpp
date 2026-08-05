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

#include <cee/gui/plot.h>
#include <object_impl.h>

#include <algorithm>

namespace cee {
namespace gui {
	constexpr float map(float n, float oldMin, float oldMax, float newMin, float newMax) {
		return ((newMax - newMin) * ((n - oldMin) / (oldMax - oldMin))) + newMin;
	}

	constexpr size_t map(size_t n, size_t oldMin, size_t oldMax, size_t newMin, size_t newMax) {
		return (static_cast<float>(newMax - newMin) * (static_cast<float>(n - oldMin) /
					static_cast<float>(oldMax - oldMin))) + newMin;
	}

	constexpr int map(int n, int oldMin, int oldMax, int newMin, int newMax) {
		return (static_cast<float>(newMax - newMin) * (static_cast<float>(n - oldMin) /
					static_cast<float>(oldMax - oldMin))) + newMin;
	}

	std::vector<Point> &Plot::PlotPoints(Plot *o,
			const std::vector<float> &data,
			std::vector<Point> &points, size_t lineBreak) {
		if (data.size() == 0)
			return points;


		Point p1, p2;
		float xMin = o->m_Impl->m_AbsoluteRect.x;
		float xMax = o->m_Impl->m_AbsoluteRect.x + o->m_Impl->m_AbsoluteRect.w;
		float yMin = o->m_Impl->m_AbsoluteRect.y;
		float yMax = o->m_Impl->m_AbsoluteRect.y + o->m_Impl->m_AbsoluteRect.h;

		if (data.size() > ((xMax - xMin) * 4))
			throw GUIError(o->GetDebugName(), "Cannot render large buffers yet");

		p1.x = map(static_cast<std::size_t>(std::floor(xMin)),
				0ul, data.size(),
				static_cast<size_t>(std::floor(xMin)),
				static_cast<size_t>(std::ceil(xMax)));
		p1.y = map(data[0],
				0.f, 1.f,
				std::ceil(yMax), std::floor(yMin));

		points.clear();
		points.reserve(data.size());
		for (std::size_t i = 1; i < data.size(); ++i) {
			p2.x = map(i,
					0ul, data.size(),
					static_cast<size_t>(std::floor(xMin)),
					static_cast<size_t>(std::ceil(xMax)));
			p2.y = map(data[i],
					0.f, 1.f,
					yMax, yMin);

			points.push_back(p1);
			p1.x = p2.x;
			p1.y = p2.y;
		}

		return points;
	}

	Plot::Plot()
	 : m_Color({ 1.f, 1.f, 1.f, 1.f }) {
	}

	Plot::Plot(const float *data, std::size_t count)
	 : m_Data(data, data + count), m_Color({ 1.f, 1.f, 1.f, 1.f }) {
	}

	Plot::Plot(const Color &color)
	 : m_Color(color) {
	}

	Plot::Plot(const float *data, std::size_t count, const Color &color)
	 : m_Data(data, data + count), m_Color(color) {
	}

	void Plot::SetData(const float *data, std::size_t count, std::size_t offset) {
		if (offset + count > m_Data.size())
			m_Data.resize(offset + count);
		std::copy_n(data, count, m_Data.begin() + offset);
		m_DataStale = true;
	}

	Rect Plot::Clip() const {
		return m_Impl->m_AbsoluteRect;
	}

	Size Plot::OnMeasure(const Constraints &c) {
		return { c.maxWidth, c.maxHeight };
	}

	void Plot::OnRender() {
		if (m_Data.size() == 0)
			return;

		if (m_DataStale) {
			PlotPoints(this, m_Data, m_Points, m_LineBreakPos);
			m_DataStale = false;
		}

		m_Impl->ctx->UseShader(Context::GuiShader::Flat);

		if (m_LineBreakPos > m_LineBreakWidth)
			m_Impl->ctx->DrawPolyLine(std::span(m_Points.begin(),
						m_Points.begin() + m_LineBreakPos), 2, m_Color);

		if (m_Points.begin() + m_LineBreakPos + m_LineBreakWidth < m_Points.end())
			m_Impl->ctx->DrawPolyLine(std::span(m_Points.begin() + m_LineBreakPos + m_LineBreakWidth,
						m_Points.end()), 2, m_Color);
	}

}
}

