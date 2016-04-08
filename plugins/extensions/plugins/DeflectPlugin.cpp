/* Copyright (c) 2011-2016, EPFL/Blue Brain Project
 * All rights reserved. Do not distribute without permission.
 * Responsible Author: Cyrille Favreau <cyrille.favreau@epfl.ch>
 *
 * This file is part of BRayns
 */

#include "DeflectPlugin.h"

#include <brayns/common/scene/Scene.h>
#include <brayns/common/renderer/FrameBuffer.h>
#include <brayns/common/camera/Camera.h>

namespace brayns
{

DeflectPlugin::DeflectPlugin(
    ApplicationParameters& applicationParameters,
    ExtensionParameters& extensionParameters )
    : ExtensionPlugin( applicationParameters, extensionParameters )
    , _theta( 0.f )
    , _phi( 0.f )
    , _previousTouchPosition( 0.5f, 0.5f, -1.f )
    , _interaction(false)
    , _compressImage( applicationParameters.getJpegCompression( ) != 100 )
    , _compressionQuality( applicationParameters.getJpegCompression( ))
    , _hostname( _applicationParameters.getDeflectHostname( ))
    , _streamName( _applicationParameters.getDeflectStreamname( ))
    , _stream( nullptr )

{
    _initializeDeflect( );
}

void DeflectPlugin::run()
{
    if( _stream->isConnected( ))
    {
        _sendDeflectFrame( );
        _handleDeflectEvents( );
    }
}

void DeflectPlugin::_initializeDeflect()
{
    BRAYNS_INFO << "Connecting to DisplayCluster on host " <<
                   _hostname << std::endl;

    _stream.reset(new deflect::Stream(_streamName, _hostname));
    if( !_stream->isConnected())
        BRAYNS_ERROR << "Could not connect to " << _hostname << std::endl;

    if( _stream && !_stream->registerForEvents( ))
        BRAYNS_ERROR << "Could not register for events!" << std::endl;
}

void DeflectPlugin::_sendDeflectFrame()
{
    const Vector2i frameSize = _extensionParameters.frameBuffer->getSize();
    _send( frameSize, (unsigned long*)
        _extensionParameters.frameBuffer->getColorBuffer(), true);
}

void DeflectPlugin::_handleDeflectEvents()
{
    HandledEvents handledEvents(
        Vector2f( 0.f, 0.f ),
        Vector2f( 0.f, 0.f ),
        false,
        false );

    if( _handleTouchEvents( handledEvents ))
    {
        if( handledEvents.pressed )
        {
            _previousTouchPosition.x() = handledEvents.position.x();
            _previousTouchPosition.y() = handledEvents.position.y();
        }
        else
        {
            if( handledEvents.position.length() >
                std::numeric_limits<float>::epsilon() ||
                handledEvents.wheelDelta.y() >
                std::numeric_limits<float>::epsilon() )
            {
                const Vector3f& center = _extensionParameters.scene->
                    getWorldBounds().getCenter();
                const Vector3f& size = _extensionParameters.scene->
                    getWorldBounds().getSize();

                const float du =
                    _previousTouchPosition.x() -
                    handledEvents.position.x();
                const float dv =
                    _previousTouchPosition.y() -
                    handledEvents.position.y();

                _theta -= std::asin( du );
                _phi += std::asin( dv );

                _previousTouchPosition.x() = handledEvents.position.x();
                _previousTouchPosition.y() = handledEvents.position.y();
                _previousTouchPosition.z() +=
                    handledEvents.wheelDelta.y() / size.z();
                _previousTouchPosition.z() =
                    std::min(0.f, _previousTouchPosition.z());

                if( du!=0.f || dv!=0.f || handledEvents.wheelDelta.y() != 0.f )
                {
                    const Vector3f cameraPosition = center + size * Vector3f(
                        _previousTouchPosition.z( ) *
                            std::cos( _phi ) *
                            std::cos( _theta ),
                        _previousTouchPosition.z( ) *
                            std::sin( _phi ) *
                            std::cos( _theta ),
                        _previousTouchPosition.z( ) *
                            std::sin( _theta ));

                   _extensionParameters.camera->setPosition( cameraPosition );
                   _extensionParameters.camera->setTarget( center );
                   _extensionParameters.frameBuffer->clear();
                }
            }
        }
    }
}

void DeflectPlugin::_send(
    const Vector2i& windowSize,
    unsigned long* imageData,
    const bool swapXAxis)
{
    if(!_stream->isConnected())
        return;

    deflect::ImageWrapper deflectImage(
                imageData, windowSize.x(), windowSize.y(), deflect::RGBA);

    deflectImage.compressionPolicy =
            _compressImage ? deflect::COMPRESSION_ON : deflect::COMPRESSION_OFF;

    deflectImage.compressionQuality = _compressionQuality;
    if( swapXAxis )
        deflect::ImageWrapper::swapYAxis(
            (void*)imageData, windowSize.x(), windowSize.y(), 4);

    bool success = _stream->send(deflectImage);
    _stream->finishFrame();

    if(!success)
    {
        if (!_stream->isConnected())
            BRAYNS_ERROR << "Stream closed, exiting." << std::endl;
        else
            BRAYNS_ERROR << "failure in deflectStreamSend()" << std::endl;
    }
}

bool DeflectPlugin::_handleTouchEvents( HandledEvents& handledEvents )
{
    if(!_stream || !_stream->isRegisteredForEvents())
        return false;

    /* increment rotation angle according to interaction, or by a constant rate
     * if interaction is not enabled. Note that mouse position is in normalized
     * window coordinates: (0,0) to (1,1)
     * Note: there is a risk of missing events since we only process the
     * latest state available. For more advanced applications, event
     * processing should be done in a separate thread.
     */
    while(_stream->hasEvent())
    {
        const deflect::Event& event = _stream->getEvent();
        if(event.type == deflect::Event::EVT_CLOSE)
        {
            BRAYNS_INFO << "Received close..." << std::endl;
            handledEvents.closeApplication = true;
        }

        handledEvents.pressed = (event.type == deflect::Event::EVT_PRESS);

        if (event.type == deflect::Event::EVT_WHEEL)
            handledEvents.wheelDelta = Vector2f(event.dx, event.dy);

        handledEvents.position = Vector2f(event.mouseX, event.mouseY);
    }
    return true;
}

}
