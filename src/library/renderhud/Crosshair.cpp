/*
    Copyright 2015-2023 Clément Gallet <clement.gallet@ens-lyon.org>

    This file is part of libTAS.

    libTAS is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    libTAS is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with libTAS.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "Crosshair.h"

#include "../external/imgui/imgui.h"

namespace libtas {

void Crosshair::draw(const AllInputs& ai)
{
    int size = 5;
    ImGui::GetBackgroundDrawList()->AddLine(ImVec2(ai.pointer_x, ai.pointer_y-size), ImVec2(ai.pointer_x, ai.pointer_y+size), IM_COL32(255, 255, 255, 255));
    ImGui::GetBackgroundDrawList()->AddLine(ImVec2(ai.pointer_x-size, ai.pointer_y), ImVec2(ai.pointer_x+size, ai.pointer_y+size), IM_COL32(255, 255, 255, 255));
}

}