/***************************************************************************
 *   Free Heroes of Might and Magic II: https://github.com/ihhub/fheroes2  *
 *   Copyright (C) 2020 - 2022                                             *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/

#include "ui_button.h"
#include "agg_image.h"
#include "dialog.h"
#include "game_hotkeys.h"
#include "icn.h"
#include "localevent.h"
#include "pal.h"
#include "settings.h"

namespace fheroes2
{
    ButtonBase::ButtonBase( int32_t offsetX, int32_t offsetY )
        : _offsetX( offsetX )
        , _offsetY( offsetY )
        , _isPressed( false )
        , _isEnabled( true )
        , _isVisible( true )
        , _releasedSprite( nullptr )
        , _disabledSprite()
    {}

    ButtonBase::ButtonBase( ButtonBase && button ) noexcept
        : ButtonBase()
    {
        _swap( button );
    }

    ButtonBase & ButtonBase::operator=( ButtonBase && button ) noexcept
    {
        if ( this != &button ) {
            _swap( button );
        }
        return *this;
    }

    void ButtonBase::_swap( ButtonBase & button )
    {
        std::swap( _offsetX, button._offsetX );
        std::swap( _offsetY, button._offsetY );
        std::swap( _isPressed, button._isPressed );
        std::swap( _isEnabled, button._isEnabled );
        std::swap( _isVisible, button._isVisible );
        std::swap( _releasedSprite, button._releasedSprite );
        std::swap( _disabledSprite, button._disabledSprite );
    }

    bool ButtonBase::isEnabled() const
    {
        return _isEnabled;
    }

    bool ButtonBase::isDisabled() const
    {
        return !_isEnabled;
    }

    bool ButtonBase::isPressed() const
    {
        return _isPressed;
    }

    bool ButtonBase::isReleased() const
    {
        return !_isPressed;
    }

    bool ButtonBase::isVisible() const
    {
        return _isVisible;
    }

    bool ButtonBase::isHidden() const
    {
        return !_isVisible;
    }

    void ButtonBase::press()
    {
        if ( isEnabled() ) {
            _isPressed = true;
            updateSubscription();
        }
    }

    void ButtonBase::release()
    {
        if ( isEnabled() ) {
            _isPressed = false;
            updateSubscription();
        }
    }

    void ButtonBase::enable()
    {
        _isEnabled = true;
        updateSubscription();
    }

    void ButtonBase::disable()
    {
        _isEnabled = false;
        _isPressed = false; // button can't be disabled and pressed
        updateSubscription();
    }

    void ButtonBase::show()
    {
        _isVisible = true;
        updateSubscription();
    }

    void ButtonBase::hide()
    {
        _isVisible = false;
        updateSubscription();
    }

    void ButtonBase::setPosition( int32_t offsetX_, int32_t offsetY_ )
    {
        _offsetX = offsetX_;
        _offsetY = offsetY_;
    }

    void ButtonBase::draw( Image & output ) const
    {
        if ( !isVisible() )
            return;

        if ( isPressed() ) {
            // button can't be disabled and pressed
            const Sprite & sprite = _getPressed();
            Blit( sprite, output, _offsetX + sprite.x(), _offsetY + sprite.y() );
        }
        else {
            const Sprite & sprite = isEnabled() ? _getReleased() : _getDisabled();
            Blit( sprite, output, _offsetX + sprite.x(), _offsetY + sprite.y() );
        }
    }

    bool ButtonBase::drawOnPress( Image & output )
    {
        if ( !isPressed() ) {
            press();
            draw( output );
            Display::instance().render( area() );
            return true;
        }
        return false;
    }

    bool ButtonBase::drawOnRelease( Image & output )
    {
        if ( isPressed() ) {
            release();
            draw( output );
            Display::instance().render( area() );
            return true;
        }
        return false;
    }

    Rect ButtonBase::area() const
    {
        const Sprite & sprite = isPressed() ? _getPressed() : _getReleased();
        return Rect( _offsetX + sprite.x(), _offsetY + sprite.y(), sprite.width(), sprite.height() );
    }

    const Sprite & ButtonBase::_getDisabled() const
    {
        const Sprite & sprite = _getReleased();
        if ( !_disabledSprite || ( _releasedSprite != &sprite ) ) {
            _releasedSprite = &sprite;
            _disabledSprite.reset( new Sprite( sprite ) );
            ApplyPalette( *_disabledSprite, PAL::GetPalette( PAL::PaletteType::DARKENING ) );
        }

        return *_disabledSprite.get();
    }

    Button::Button( int32_t offsetX, int32_t offsetY )
        : ButtonBase( offsetX, offsetY )
        , _icnId( -1 )
        , _releasedIndex( 0 )
        , _pressedIndex( 0 )
    {}

    Button::Button( int32_t offsetX, int32_t offsetY, int icnId, uint32_t releasedIndex, uint32_t pressedIndex )
        : ButtonBase( offsetX, offsetY )
        , _icnId( icnId )
        , _releasedIndex( releasedIndex )
        , _pressedIndex( pressedIndex )
    {}

    void Button::setICNInfo( int icnId, uint32_t releasedIndex, uint32_t pressedIndex )
    {
        _icnId = icnId;
        _releasedIndex = releasedIndex;
        _pressedIndex = pressedIndex;
    }

    const Sprite & Button::_getPressed() const
    {
        return AGG::GetICN( _icnId, _pressedIndex );
    }

    const Sprite & Button::_getReleased() const
    {
        return AGG::GetICN( _icnId, _releasedIndex );
    }

    ButtonSprite::ButtonSprite( int32_t offsetX, int32_t offsetY )
        : ButtonBase( offsetX, offsetY )
    {}

    ButtonSprite::ButtonSprite( int32_t offsetX, int32_t offsetY, const Sprite & released, const Sprite & pressed, const Sprite & disabled )
        : ButtonBase( offsetX, offsetY )
        , _released( released )
        , _pressed( pressed )
        , _disabled( disabled )
    {}

    ButtonSprite::ButtonSprite( ButtonSprite && button ) noexcept
        : ButtonBase( std::move( button ) )
    {
        std::swap( _released, button._released );
        std::swap( _pressed, button._pressed );
        std::swap( _disabled, button._disabled );
    }

    ButtonSprite & ButtonSprite::operator=( ButtonSprite && button ) noexcept
    {
        if ( this != &button ) {
            ButtonBase::_swap( button );
            std::swap( _released, button._released );
            std::swap( _pressed, button._pressed );
            std::swap( _disabled, button._disabled );
        }
        return *this;
    }

    void ButtonSprite::setSprite( const Sprite & released, const Sprite & pressed, const Sprite & disabled )
    {
        _released = released;
        _pressed = pressed;
        _disabled = disabled;
    }

    const Sprite & ButtonSprite::_getPressed() const
    {
        return _pressed;
    }

    const Sprite & ButtonSprite::_getReleased() const
    {
        return _released;
    }

    const Sprite & ButtonSprite::_getDisabled() const
    {
        if ( _disabled.empty() ) {
            return ButtonBase::_getDisabled();
        }

        return _disabled;
    }

    ButtonGroup::ButtonGroup( const Rect & area, int buttonTypes )
    {
        const int icnId = Settings::Get().ExtGameEvilInterface() ? ICN::SYSTEME : ICN::SYSTEM;

        Point offset;

        switch ( buttonTypes ) {
        case Dialog::YES | Dialog::NO:
            offset.x = area.x;
            offset.y = area.y + area.height - AGG::GetICN( icnId, 5 ).height();
            createButton( offset.x, offset.y, icnId, 5, 6, Dialog::YES );

            offset.x = area.x + area.width - AGG::GetICN( icnId, 7 ).width();
            offset.y = area.y + area.height - AGG::GetICN( icnId, 7 ).height();
            createButton( offset.x, offset.y, icnId, 7, 8, Dialog::NO );
            break;

        case Dialog::OK | Dialog::CANCEL:
            offset.x = area.x;
            offset.y = area.y + area.height - AGG::GetICN( icnId, 1 ).height();
            createButton( offset.x, offset.y, icnId, 1, 2, Dialog::OK );

            offset.x = area.x + area.width - AGG::GetICN( icnId, 3 ).width();
            offset.y = area.y + area.height - AGG::GetICN( icnId, 3 ).height();
            createButton( offset.x, offset.y, icnId, 3, 4, Dialog::CANCEL );
            break;

        case Dialog::OK:
            offset.x = area.x + ( area.width - AGG::GetICN( icnId, 1 ).width() ) / 2;
            offset.y = area.y + area.height - AGG::GetICN( icnId, 1 ).height();
            createButton( offset.x, offset.y, icnId, 1, 2, Dialog::OK );
            break;

        case Dialog::CANCEL:
            offset.x = area.x + ( area.width - AGG::GetICN( icnId, 3 ).width() ) / 2;
            offset.y = area.y + area.height - AGG::GetICN( icnId, 3 ).height();
            createButton( offset.x, offset.y, icnId, 3, 4, Dialog::CANCEL );
            break;

        default:
            break;
        }
    }

    ButtonGroup::~ButtonGroup()
    {
        for ( size_t i = 0; i < _button.size(); ++i ) {
            delete _button[i];
        }

        _button.clear();
        _value.clear();
    }

    void ButtonGroup::createButton( int32_t offsetX, int32_t offsetY, int icnId, uint32_t releasedIndex, uint32_t pressedIndex, int returnValue )
    {
        _button.push_back( new Button( offsetX, offsetY, icnId, releasedIndex, pressedIndex ) );
        _value.emplace_back( returnValue );
    }

    void ButtonGroup::createButton( int32_t offsetX, int32_t offsetY, const Sprite & released, const Sprite & pressed, int returnValue )
    {
        _button.push_back( new ButtonSprite( offsetX, offsetY, released, pressed ) );
        _value.emplace_back( returnValue );
    }

    void ButtonGroup::addButton( ButtonSprite && button, int returnValue )
    {
        _button.push_back( new ButtonSprite( std::move( button ) ) );
        _value.emplace_back( returnValue );
    }

    void ButtonGroup::draw( Image & area ) const
    {
        for ( size_t i = 0; i < _button.size(); ++i ) {
            _button[i]->draw( area );
        }
    }

    ButtonBase & ButtonGroup::button( size_t id )
    {
        return *_button[id];
    }

    const ButtonBase & ButtonGroup::button( size_t id ) const
    {
        return *_button[id];
    }

    size_t ButtonGroup::size() const
    {
        return _button.size();
    }

    int ButtonGroup::processEvents()
    {
        LocalEvent & le = LocalEvent::Get();

        for ( size_t i = 0; i < _button.size(); ++i ) {
            if ( _button[i]->isEnabled() ) {
                le.MousePressLeft( _button[i]->area() ) ? _button[i]->drawOnPress() : _button[i]->drawOnRelease();
            }
        }

        for ( size_t i = 0; i < _button.size(); ++i ) {
            if ( _button[i]->isEnabled() && le.MouseClickLeft( _button[i]->area() ) ) {
                return _value[i];
            }
        }

        for ( size_t i = 0; i < _button.size(); ++i ) {
            if ( _button[i]->isEnabled() ) {
                if ( ( _value[i] == Dialog::YES || _value[i] == Dialog::OK ) && Game::HotKeyPressEvent( Game::HotKeyEvent::DEFAULT_OKAY ) ) {
                    return _value[i];
                }
                if ( ( _value[i] == Dialog::CANCEL || _value[i] == Dialog::NO ) && Game::HotKeyPressEvent( Game::HotKeyEvent::DEFAULT_CANCEL ) ) {
                    return _value[i];
                }
            }
        }

        return Dialog::ZERO;
    }

    ButtonRestorer::ButtonRestorer( ButtonBase & button, Image & area )
        : _button( button )
        , _area( area )
        , _isDisabled( button.isDisabled() )
    {
        if ( !_isDisabled ) {
            _button.disable();
            _button.draw( _area );
        }
    }

    ButtonRestorer::~ButtonRestorer()
    {
        if ( !_isDisabled ) {
            _button.enable();
            _button.draw( _area );
        }
    }

    void OptionButtonGroup::addButton( ButtonBase * button )
    {
        if ( button == nullptr )
            return;

        _button.push_back( button );
        button->subscribe( this );
    }

    void OptionButtonGroup::draw( Image & area ) const
    {
        for ( size_t i = 0; i < _button.size(); ++i ) {
            _button[i]->draw( area );
        }
    }

    void OptionButtonGroup::senderUpdate( const ActionObject * sender )
    {
        if ( sender == nullptr ) // how is it even possible?
            return;

        for ( size_t i = 0; i < _button.size(); ++i ) {
            if ( sender == _button[i] ) {
                const ButtonBase * button = _button[i];
                if ( button->isPressed() ) {
                    unsubscribeAll();

                    for ( size_t buttonId = 0; buttonId < _button.size(); ++buttonId ) {
                        if ( i != buttonId ) {
                            _button[buttonId]->release();
                        }
                    }

                    subscribeAll();
                }
            }
        }
    }

    void OptionButtonGroup::subscribeAll()
    {
        for ( size_t i = 0; i < _button.size(); ++i ) {
            _button[i]->subscribe( this );
        }
    }

    void OptionButtonGroup::unsubscribeAll()
    {
        for ( size_t i = 0; i < _button.size(); ++i ) {
            _button[i]->unsubscribe();
        }
    }

    ButtonSprite makeButtonWithBackground( int32_t offsetX, int32_t offsetY, const Sprite & released, const Sprite & pressed, const Image & background )
    {
        const Sprite croppedBackground = Crop( background, offsetX, offsetY, released.width(), released.height() );

        Sprite releasedWithBackground( croppedBackground.width(), croppedBackground.height(), 0, 0 );
        Copy( croppedBackground, releasedWithBackground );
        Blit( released, releasedWithBackground, released.x(), released.y() );

        Sprite pressedWithBackground( croppedBackground.width(), croppedBackground.height(), 0, 0 );
        Copy( croppedBackground, pressedWithBackground );
        Blit( pressed, pressedWithBackground, pressed.x(), pressed.y() );

        Sprite disabled( released );
        ApplyPalette( disabled, PAL::GetPalette( PAL::PaletteType::DARKENING ) );

        Sprite disabledWithBackground( croppedBackground.width(), croppedBackground.height(), 0, 0 );
        Copy( croppedBackground, disabledWithBackground );
        disabledWithBackground.setPosition( 0, 0 );
        Blit( disabled, disabledWithBackground, disabled.x(), disabled.y() );

        return { offsetX, offsetY, releasedWithBackground, pressedWithBackground, disabledWithBackground };
    }

    ButtonSprite makeButtonWithShadow( int32_t offsetX, int32_t offsetY, const Sprite & released, const Sprite & pressed, const Image & background,
                                       const Point & shadowOffset )
    {
        const Sprite & shadow = fheroes2::makeShadow( released, shadowOffset, 3 );

        Sprite croppedBackground = Crop( background, offsetX + shadow.x(), offsetY + shadow.y(), shadow.width(), shadow.height() );
        Blit( shadow, croppedBackground );

        Sprite releasedWithBackground( croppedBackground.width(), croppedBackground.height(), 0, 0 );
        Copy( croppedBackground, releasedWithBackground );
        Blit( released, releasedWithBackground, released.x() - shadow.x(), released.y() - shadow.y() );

        Sprite pressedWithBackground( croppedBackground.width(), croppedBackground.height(), 0, 0 );
        Copy( croppedBackground, pressedWithBackground );
        Blit( pressed, pressedWithBackground, pressed.x() - shadow.x(), pressed.y() - shadow.y() );

        Sprite disabled( released );
        ApplyPalette( disabled, PAL::GetPalette( PAL::PaletteType::DARKENING ) );

        Sprite disabledWithBackground( croppedBackground.width(), croppedBackground.height(), 0, 0 );
        Copy( croppedBackground, disabledWithBackground );
        disabledWithBackground.setPosition( 0, 0 );
        Blit( disabled, disabledWithBackground, disabled.x() - shadow.x(), disabled.y() - shadow.y() );

        return { offsetX + shadow.x(), offsetY + shadow.y(), releasedWithBackground, pressedWithBackground, disabledWithBackground };
    }
}
