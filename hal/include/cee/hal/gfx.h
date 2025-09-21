/*
 * CeeHealth
 * Copyright (C) 2025 Chloe Eather
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

#ifndef CEE_HAL_GFX_H_
#define CEE_HAL_GFX_H_

#if defined(__cplusplus)
extern "C" {
#endif

typedef void(* PFNSwapBuffers)(void* userData);

struct HALGfx;

struct HALGfx *HALGfxCreate(void);
int HALGfxInit(struct HALGfx *gfx);
int HALGfxShutdown(struct HALGfx *gfx);
void HALGfxDestroy(struct HALGfx *gfx);

int HALGfxGetWidth(const struct HALGfx *gfx);
int HALGfxGetHeight(const struct HALGfx *gfx);

int HALGfxPageFlip(struct HALGfx *gfx);

const char *HALGfxGetVersionString(struct HALGfx *gfx);

#if defined(__cplusplus)
}
#endif

#endif

