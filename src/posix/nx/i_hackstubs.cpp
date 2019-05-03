// Emacs style mode select -*- C++ -*-
//---------------------------------------------------------------------------
//
// Copyright(C) 2000 Simon Howard
// Copyright(C) 2005-2008 Christoph Oelckers
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//
//--------------------------------------------------------------------------

// HACK: when you disable --gc-sections, which is required for the *reg
//       sections to work correctly, an unused Mesa function from libEGL
//       starts complaining about undefined references because it's not GC'd
//       these are stubs to replace whatever it complains about

extern "C"
{
void spirv_to_nir(void) { }
void nir_lower_constant_initializers(void) { }
void nir_lower_returns(void) { }
void nir_inline_functions(void) { }
void nir_opt_deref(void) { }
void nir_split_var_copies(void) { }
void nir_split_per_member_structs(void) { }
void nir_remap_dual_slot_attributes(void) { }
}
