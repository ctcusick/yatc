//////////////////////////////////////////////////////////////////////
// Yet Another Tibia Client
//////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software Foundation,
// Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
//////////////////////////////////////////////////////////////////////

#include "enginesdl.h"
#include "spritesdl.h"

SpriteSDL::SpriteSDL(const std::string& filename, int index) : Sprite(filename, index)
{

}

SpriteSDL::~SpriteSDL()
{
	//
}


void SpriteSDL::Blit(float dx, float dy, float sx, float sy, float w, float h)
{
	if(!getImage())
		return;
	// code is like this because of dx5.0 ... see docs/html/sdlrect.html in SDL documentation for more info
	SDL_Rect src = {(int)sx,(int)sy,(int)(w),(int)(h)};
	SDL_Rect dst = {(int)dx,(int)dy,(int)(w),(int)(h)};
	while(SDL_BlitSurface(getImage(), &src, g_engine->m_screen, &dst) == -2){
		while(SDL_LockSurface(getImage()) < 0 ){
				SDL_Delay(10);
		}
		SDL_UnlockSurface(getImage());
	}
}




void SpriteSDL::Blit(float dx, float dy, float sx, float sy, float w, float h, float destw, float desth)
{
	// TODO (ivucica#1#) this is COMPLETELY INCORRECT, this implementation TILES the image while it should STRETCH it instead
	if(!getImage())
		return;

/*	for (float i=0; i<destw; i+=w)
		for (float j=0;j<desth;j+=h)
			Blit(dx+i, dy+j, sx, sy, w, h);*/

	Stretch(destw, desth, true);
	Blit(dx,dy,sx,sy,w,h);
}
