/*
    Copyright 2015-2020 Clément Gallet <clement.gallet@ens-lyon.org>

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

#include "Input.h"
#include "../../shared/SingleInput.h"

#include <iostream>
#include <lua.h>
#include <lauxlib.h>

static AllInputs* ai;

/* List of functions to register */
static const luaL_Reg input_functions[] =
{
    { "clear", Lua::Input::clear},
    { "setKey", Lua::Input::setKey},
    { "getKey", Lua::Input::getKey},
    { "setMouseCoords", Lua::Input::setMouseCoords},
    { "getMouseCoords", Lua::Input::getMouseCoords},
    { "setMouseButtons", Lua::Input::setMouseButtons},
    { "getMouseButtons", Lua::Input::getMouseButtons},
    { "setControllerButton", Lua::Input::setControllerButton},
    { "getControllerButton", Lua::Input::getControllerButton},
    { "setControllerAxis", Lua::Input::setControllerAxis},
    { "getControllerAxis", Lua::Input::getControllerAxis},
    { NULL, NULL }
};

void Lua::Input::registerFunctions(lua_State *L)
{
    luaL_newlib(L, input_functions);
    lua_setglobal(L, "input");
}

void Lua::Input::onInput(Context* context, AllInputs* frame_ai)
{
    ai = frame_ai;
    
    lua_getglobal(context->lua_state, "onInput");
    if (lua_isfunction(context->lua_state, -1)) {
        int ret = lua_pcall(context->lua_state, 0, 0, 0);
        if (ret != 0) {
            std::cerr << "error running function `onInput': " << lua_tostring(context->lua_state, -1) << std::endl;
            lua_pop(context->lua_state, 1);  // pop error message from the stack
        }
    }
    else {
        /* No function, we need to clear the stack */
        lua_pop(context->lua_state, 1);
    }
}

int Lua::Input::clear(lua_State *L)
{
    ai->emptyInputs();
    return 0;
}

int Lua::Input::setKey(lua_State *L)
{
    int keysym = static_cast<int>(lua_tonumber(L, 1));
    int state = static_cast<int>(lua_tonumber(L, 2));
    
    SingleInput si = {SingleInput::IT_KEYBOARD, keysym, ""};
    ai->setInput(si, state);
    return 0;
}

int Lua::Input::getKey(lua_State *L)
{
    int keysym = static_cast<int>(lua_tonumber(L, 1));

    SingleInput si = {SingleInput::IT_KEYBOARD, keysym, ""};
    lua_pushnumber(L, static_cast<double>(ai->getInput(si)));
    return 1;
}

int Lua::Input::setMouseCoords(lua_State *L)
{
    int x = static_cast<int>(lua_tonumber(L, 1));
    int y = static_cast<int>(lua_tonumber(L, 2));
    int mode = static_cast<int>(lua_tonumber(L, 3));
    
    SingleInput si = {SingleInput::IT_POINTER_X, 0, ""};
    ai->setInput(si, x);
    si = {SingleInput::IT_POINTER_Y, 0, ""};
    ai->setInput(si, y);
    si = {SingleInput::IT_POINTER_MODE, 0, ""};
    ai->setInput(si, mode);
    return 0;
}

int Lua::Input::getMouseCoords(lua_State *L)
{
    SingleInput si = {SingleInput::IT_POINTER_X, 0, ""};
    lua_pushnumber(L, static_cast<double>(ai->getInput(si)));
    si = {SingleInput::IT_POINTER_Y, 0, ""};
    lua_pushnumber(L, static_cast<double>(ai->getInput(si)));
    si = {SingleInput::IT_POINTER_MODE, 0, ""};
    lua_pushnumber(L, static_cast<double>(ai->getInput(si)));
    return 3;
}

int Lua::Input::setMouseButtons(lua_State *L)
{
    int button = static_cast<int>(lua_tonumber(L, 1));
    int state = static_cast<int>(lua_tonumber(L, 2));
    
    SingleInput si = {SingleInput::IT_POINTER_B1 + button, 0, ""};
    ai->setInput(si, state);
    return 0;
}

int Lua::Input::getMouseButtons(lua_State *L)
{
    int button = static_cast<int>(lua_tonumber(L, 1));

    SingleInput si = {SingleInput::IT_POINTER_B1 + button, 0, ""};
    lua_pushnumber(L, static_cast<double>(ai->getInput(si)));
    return 1;
}

int Lua::Input::setControllerButton(lua_State *L)
{
    int controller = static_cast<int>(lua_tonumber(L, 1));
    int button = static_cast<int>(lua_tonumber(L, 2));
    int state = static_cast<int>(lua_tonumber(L, 3));
    
    SingleInput si = {(controller << SingleInput::IT_CONTROLLER_ID_SHIFT) | button, 0, ""};
    ai->setInput(si, state);
    return 0;
}

int Lua::Input::getControllerButton(lua_State *L)
{
    int controller = static_cast<int>(lua_tonumber(L, 1));
    int button = static_cast<int>(lua_tonumber(L, 2));
    
    SingleInput si = {(controller << SingleInput::IT_CONTROLLER_ID_SHIFT) | button, 0, ""};
    lua_pushnumber(L, static_cast<double>(ai->getInput(si)));
    return 1;
}

int Lua::Input::setControllerAxis(lua_State *L)
{
    int controller = static_cast<int>(lua_tonumber(L, 1));
    int axis = static_cast<int>(lua_tonumber(L, 2));
    short value = static_cast<short>(lua_tonumber(L, 3));
    
    SingleInput si = {(controller << SingleInput::IT_CONTROLLER_ID_SHIFT) | SingleInput::IT_CONTROLLER_AXIS_MASK | axis, 0, ""};
    ai->setInput(si, value);
    return 0;
}

int Lua::Input::getControllerAxis(lua_State *L)
{
    int controller = static_cast<int>(lua_tonumber(L, 1));
    int axis = static_cast<int>(lua_tonumber(L, 2));
    
    SingleInput si = {(controller << SingleInput::IT_CONTROLLER_ID_SHIFT) | SingleInput::IT_CONTROLLER_AXIS_MASK | axis, 0, ""};
    lua_pushnumber(L, static_cast<double>(ai->getInput(si)));
    return 1;
}
