#ifndef XVIDEORENDERER_H
#define XVIDEORENDERER_H
#include <QMap>
#include <QString>
#include <QtAV>
#include <QtAVWidgets>
enum XVideoRenderer {
    GLWidget2,
    OpenGLWidget,
    Direct2D,
    GDI,
    GLWidget,
    GraphicsItem,
    OpenGLWindow,
    Widget,
    X11,
    XV
};
const QMap<QString, XVideoRenderer> XVideoRendererMap =
{
    {"GLWidget2", XVideoRenderer::GLWidget2},
    {"OpenGLWidget", XVideoRenderer::OpenGLWidget},
    {"Direct2D", XVideoRenderer::Direct2D},
    {"GDI", XVideoRenderer::GDI},
    {"GLWidget", XVideoRenderer::GLWidget},
    {"GraphicsItem", XVideoRenderer::GraphicsItem},
    {"OpenGLWindow", XVideoRenderer::OpenGLWindow},
    {"Widget", XVideoRenderer::Widget},
    {"X11", XVideoRenderer::X11},
    {"XV", XVideoRenderer::XV},
};
const QMap<XVideoRenderer, QString> XVideoRendererReverseMap =
{
    {XVideoRenderer::GLWidget2, "GLWidget2"},
    {XVideoRenderer::OpenGLWidget, "OpenGLWidget"},
    {XVideoRenderer::Direct2D, "Direct2D"},
    {XVideoRenderer::GDI, "GDI", },
    {XVideoRenderer::GLWidget, "GLWidget", },
    {XVideoRenderer::GraphicsItem, "GraphicsItem"},
    {XVideoRenderer::OpenGLWindow, "OpenGLWindow"},
    {XVideoRenderer::Widget, "Widget"},
    {XVideoRenderer::X11, "X11"},
    {XVideoRenderer::XV, "XV"},
};
const QMap<XVideoRenderer, int> QtAVVideoRendererIdMap =
{
    {XVideoRenderer::GLWidget2, QtAV::VideoRendererId_GLWidget2},
    {XVideoRenderer::OpenGLWidget, QtAV::VideoRendererId_OpenGLWidget},
    {XVideoRenderer::Direct2D, QtAV::VideoRendererId_Direct2D},
    {XVideoRenderer::GDI, QtAV::VideoRendererId_GDI},
    {XVideoRenderer::GLWidget, QtAV::VideoRendererId_GLWidget},
    {XVideoRenderer::GraphicsItem, QtAV::VideoRendererId_GraphicsItem},
    {XVideoRenderer::OpenGLWindow, QtAV::VideoRendererId_OpenGLWindow},
    {XVideoRenderer::Widget, QtAV::VideoRendererId_Widget},
    {XVideoRenderer::X11, QtAV::VideoRendererId_X11},
    {XVideoRenderer::XV, QtAV::VideoRendererId_XV}
};
#endif // XVIDEORENDERER_H
