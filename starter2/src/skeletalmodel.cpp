#include "skeletalmodel.h"
#include <cassert>

#include "starter2_util.h"
#include "vertexrecorder.h"

using namespace std;

SkeletalModel::SkeletalModel() {
    program = compileProgram(c_vertexshader, c_fragmentshader_light);
    if (!program) {
        printf("Cannot compile program\n");
        assert(false);
    }
}

SkeletalModel::~SkeletalModel() {
    // destructor will release memory when SkeletalModel is deleted
    while (m_joints.size()) {
        delete m_joints.back();
        m_joints.pop_back();
    }

    glDeleteProgram(program);
}

void SkeletalModel::load(const char *skeletonFile, const char *meshFile, const char *attachmentsFile)
{
    loadSkeleton(skeletonFile);

    m_mesh.load(meshFile);
    m_mesh.loadAttachments(attachmentsFile, (int)m_joints.size());

    computeBindWorldToJointTransforms();
    updateCurrentJointToWorldTransforms();
}

void SkeletalModel::draw(const Camera& camera, bool skeletonVisible)
{
    // draw() gets called whenever a redraw is required
    // (after an update() occurs, when the camera moves, the window is resized, etc)

    m_matrixStack.clear();

    glUseProgram(program);
    updateShadingUniforms();
    if (skeletonVisible)
    {
        drawJoints(camera);
        drawSkeleton(camera);
    }
    else
    {
        // Tell the mesh to draw itself.
        // Since we transform mesh vertices on the CPU,
        // There is no need to set a Model matrix as uniform
        camera.SetUniforms(program, Matrix4f::identity());
        m_mesh.draw();
    }
    glUseProgram(0);
}

void SkeletalModel::updateShadingUniforms() {
    // UPDATE MATERIAL UNIFORMS
    GLfloat diffColor[] = { 0.4f, 0.4f, 0.4f, 1 };
    GLfloat specColor[] = { 0.9f, 0.9f, 0.9f, 1 };
    GLfloat shininess[] = { 50.0f };
    int loc = glGetUniformLocation(program, "diffColor");
    glUniform4fv(loc, 1, diffColor);
    loc = glGetUniformLocation(program, "specColor");
    glUniform4fv(loc, 1, specColor);
    loc = glGetUniformLocation(program, "shininess");
    glUniform1f(loc, shininess[0]);

    // UPDATE LIGHT UNIFORMS
    GLfloat lightPos[] = { 3.0f, 3.0f, 5.0f, 1.0f };
    loc = glGetUniformLocation(program, "lightPos");
    glUniform4fv(loc, 1, lightPos);

    GLfloat lightDiff[] = { 120.0f, 120.0f, 120.0f, 1.0f };
    loc = glGetUniformLocation(program, "lightDiff");
    glUniform4fv(loc, 1, lightDiff);
}

void SkeletalModel::loadSkeleton(const char* filename)
{
    // Load the skeleton from file here.
    string line;
    ifstream file(filename);
    if (file.is_open()) {
        while (getline(file, line)) {
            stringstream ss(line);
            float x, y, z;
            int parent;
            ss >> x >> y >> z >> parent;
            Joint* joint = new Joint;
            joint->transform = Matrix4f::translation(x, y, z);
            if (parent == -1) {
                m_rootJoint = joint;
                m_joints.push_back(joint);
            }
            else {
                Joint* jparent = m_joints[parent];
                jparent->children.push_back(joint);
                m_joints.push_back(joint);
            }
        }
        file.close();
    }
}

void SkeletalModel::drawJointSphere(Joint* node, MatrixStack stack, const Camera& camera) {
    if (node->children.size() == 0) {
        stack.push(node->transform);
        camera.SetUniforms(program, stack.top());
        drawSphere(0.025f, 12, 12);
        stack.pop();
    } else {
        stack.push(node->transform);
        for (unsigned i = 0; i < node->children.size(); ++i) {
            drawJointSphere(node->children[i], stack, camera);
        }
        camera.SetUniforms(program, stack.top());
        drawSphere(0.025f, 12, 12);
        stack.pop();
    }
}

void SkeletalModel::drawJoints(const Camera& camera)
{
    // Draw a sphere at each joint. You will need to add a recursive
    // helper function to traverse the joint hierarchy.
    //
    // We recommend using drawSphere( 0.025f, 12, 12 )
    // to draw a sphere of reasonable size.
    //
    // You should use your MatrixStack class. A function
    // should push it's changes onto the stack, and
    // use stack.pop() to revert the stack to the original
    // state.
    drawJointSphere(m_rootJoint, m_matrixStack, camera);
}

void SkeletalModel::drawSkeletonBone(Joint* node, MatrixStack stack, const Camera& camera) {
    if (node->children.size()) {
        stack.push(node->transform);
        for (unsigned i = 0; i < node->children.size(); ++i) {
            Vector3f y = node->children[i]->transform.getCol(3).xyz();
            float height = y.abs();
            y.normalize();
            Vector3f z = Vector3f::cross(y, Vector3f(0, 0, 1)).normalized();
            Vector3f x = Vector3f::cross(z, y).normalized();
            Matrix4f R = Matrix4f(Vector4f(x, 0), Vector4f(y, 0), Vector4f(z, 0), Vector4f(0, 0, 0, 1));
            m_matrixStack.push(R);
            camera.SetUniforms(program, stack.top() * R);
            drawCylinder(6, 0.02f, height);
            drawSkeletonBone(node->children[i], stack, camera);
            m_matrixStack.pop();
        }
        stack.pop();
    }
}

void SkeletalModel::drawSkeleton(const Camera& camera)
{
    // Draw cylinders between the joints. You will need to add a recursive 
    // helper function to traverse the joint hierarchy.
    //
    // We recommend using drawCylinder(6, 0.02f, <height>);
    // to draw a cylinder of reasonable diameter.
    drawSkeletonBone(m_rootJoint, m_matrixStack, camera);
}

void SkeletalModel::setJointTransform(int jointIndex, float rX, float rY, float rZ)
{
    // Set the rotation part of the joint's transformation matrix based on the passed in Euler angles.
    Matrix3f Rx = Matrix3f::rotateX(rX);
    Matrix3f Ry = Matrix3f::rotateY(rY);
    Matrix3f Rz = Matrix3f::rotateZ(rZ);
    Matrix3f R = Rx * Ry * Rz;
    m_joints[jointIndex]->transform.setSubmatrix3x3(0, 0, R);
}

void BindWorld2World(Joint* joint, MatrixStack stack) {
    if (joint->children.size() == 0) {
        stack.push(joint->transform.inverse());
        joint->bindWorldToJointTransform = stack.top();
        stack.pop();
    } else {
        stack.push(joint->transform.inverse());
        for (unsigned i = 0; i < joint->children.size(); ++i) {
            BindWorld2World(joint->children[i], stack);
        }
        joint->bindWorldToJointTransform = stack.top();
        stack.pop();
    }
}

void SkeletalModel::computeBindWorldToJointTransforms()
{
    // 2.3.1. Implement this method to compute a per-joint transform from
    // world-space to joint space in the BIND POSE.
    //
    // Note that this needs to be computed only once since there is only
    // a single bind pose.
    //
    // This method should update each joint's bindWorldToJointTransform.
    // You will need to add a recursive helper function to traverse the joint hierarchy.
    BindWorld2World(m_rootJoint, m_matrixStack);
}

void World2BindWorld(Joint* joint, MatrixStack stack) {
    if (joint->children.size() == 0) {
        stack.push(joint->transform);
        joint->currentJointToWorldTransform = stack.top();
        stack.pop();
    } else {
        stack.push(joint->transform);
        for (unsigned i = 0; i < joint->children.size(); ++i) {
            World2BindWorld(joint->children[i], stack);
        }
        joint->currentJointToWorldTransform = stack.top();
        stack.pop();
    }
}

void SkeletalModel::updateCurrentJointToWorldTransforms()
{
    // 2.3.2. Implement this method to compute a per-joint transform from
    // joint space to world space in the CURRENT POSE.
    //
    // The current pose is defined by the rotations you've applied to the
    // joints and hence needs to be *updated* every time the joint angles change.
    //
    // This method should update each joint's currentJointToWorldTransform.
    // You will need to add a recursive helper function to traverse the joint hierarchy.
    World2BindWorld(m_rootJoint, m_matrixStack);
}

void SkeletalModel::updateMesh()
{
    // 2.3.2. This is the core of SSD.
    // Implement this method to update the vertices of the mesh
    // given the current state of the skeleton.
    // You will need both the bind pose world --> joint transforms.
    // and the current joint --> world transforms.
    for (unsigned i = 0; i < m_mesh.currentVertices.size(); ++i) {
        vector<float> w = m_mesh.attachments[i];
        Vector4f bind = Vector4f(m_mesh.bindVertices[i], 1);
        Vector4f position = Vector4f(0.0, 0.0, 0.0, 0.0);
        for (unsigned j = 0; j < m_joints.size(); ++j) {
            Vector4f tran = m_joints[j]->currentJointToWorldTransform * m_joints[j]->bindWorldToJointTransform * bind;
            position = position + w[j] * tran;
        }
        m_mesh.currentVertices[i] = position.xyz();
    }
}

