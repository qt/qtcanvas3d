/****************************************************************************
**
** Copyright (C) 2014 Digia Plc and/or its subsidiary(-ies).
** Contact: http://www.qt-project.org/legal
**
** This file is part of the QtCanvas3D module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and Digia.  For licensing terms and
** conditions see http://qt.digia.com/licensing.  For further information
** use the contact form at http://qt.digia.com/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 3 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPLv3 included in the
** packaging of this file.  Please review the following information to
** ensure the GNU Lesser General Public License version 3 requirements
** will be met: https://www.gnu.org/licenses/lgpl.html.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 2.0 or later as published by the Free 
** Software Foundation and appearing in the file LICENSE.GPL included in 
** the packaging of this file.  Please review the following information to
** ensure the GNU General Public License version 2.0 requirements will be
** met: http://www.gnu.org/licenses/gpl-2.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "program3d_p.h"

QT_CANVAS3D_BEGIN_NAMESPACE

/*!
 * \qmltype Program3D
 * \since QtCanvas3D 1.0
 * \ingroup qtcanvas3d-qml-types
 * \brief Contains a shader program.
 *
 * An uncreatable QML type that contains a compiled shader program. You can get it by calling
 * \l{Context3D::createProgram()}{Context3D.createProgram()} method.
 */

/*!
 * \internal
 */
CanvasProgram::CanvasProgram(QObject *parent) :
    CanvasAbstractObject(parent),
    m_program(new QOpenGLShaderProgram(this))
{
    initializeOpenGLFunctions();
}

/*!
 * \internal
 */
CanvasProgram::~CanvasProgram()
{
    delete m_program;
}

/*!
 * \internal
 */
int CanvasProgram::uniformLocation(const QString &name)
{
    if (!m_program)
        return -1;

    return m_program->uniformLocation(name);
}

/*!
 * \internal
 */
int CanvasProgram::attributeLocation(const QString &name)
{
    if (!m_program)
        return -1;

    return m_program->attributeLocation(name);
}

/*!
 * \internal
 */
bool CanvasProgram::isAlive()
{
    return bool(m_program);
}

/*!
 * \internal
 */
void CanvasProgram::attach(CanvasShader *shader)
{
    if (m_attachedShaders.count(shader) == 0) {
        m_attachedShaders.append(shader);
        m_program->addShader(shader->qOGLShader());
    }
}

/*!
 * \internal
 */
void CanvasProgram::detach(CanvasShader *shader)
{
    if (m_attachedShaders.count(shader) > 0) {
        m_attachedShaders.removeOne(shader);
        m_program->removeShader(shader->qOGLShader());
    }
}

/*!
 * \internal
 */
const QList<CanvasShader *> &CanvasProgram::attachedShaders() const
{
    return m_attachedShaders;
}

/*!
 * \internal
 */
void CanvasProgram::link()
{
    if (m_program)
        m_program->link();
}

/*!
 * \internal
 */
bool CanvasProgram::isLinked()
{
    if (!m_program)
        return false;

    return m_program->isLinked();
}

/*!
 * \internal
 */
void CanvasProgram::bind()
{
    if (m_program)
        m_program->bind();
}

/*!
 * \internal
 */
void CanvasProgram::bindAttributeLocation(int index, const QString &name)
{
    if (!m_program)
        return;

    m_program->bindAttributeLocation(name, index);
}

/*!
 * \internal
 */
void CanvasProgram::del()
{
    delete m_program;
    m_program = 0;
    m_attachedShaders.clear();
}

/*!
 * \internal
 */
void CanvasProgram::validateProgram()
{
    if (m_program)
        return;
    glValidateProgram(m_program->programId());
}

/*!
 * \internal
 */
int CanvasProgram::id()
{
    if (!m_program)
        return -1;

    return m_program->programId();
}

/*!
 * \internal
 */
QString CanvasProgram::log()
{
    if (!m_program)
        return "";

    return m_program->log();
}

/*!
 * \internal
 */
QDebug operator<<(QDebug dbg, const CanvasProgram *program)
{
    if (program)
        dbg.nospace() << "Program3D("<< program->name() << ", id:" << program->m_program->programId() << ")";
    else
        dbg.nospace() << "Program3D("<< ((void*) program) <<")";
    return dbg.maybeSpace();
}

QT_CANVAS3D_END_NAMESPACE
