#include "matrixstack.h"

MatrixStack::MatrixStack()
{
	// Initialize the matrix stack with the identity matrix.
    m_matrices.clear();
    m_matrices.push_back(Matrix4f(1, 0, 0, 0,
                                  0, 1, 0, 0,
                                  0, 0, 1, 0,
                                  0, 0, 0, 1));
}

void MatrixStack::clear()
{
	// Revert to just containing the identity matrix.
    m_matrices.clear();
    m_matrices.push_back(Matrix4f(1, 0, 0, 0,
                                  0, 1, 0, 0,
                                  0, 0, 1, 0,
                                  0, 0, 0, 1));
}

Matrix4f MatrixStack::top()
{
	// Return the top of the stack
	// return Matrix4f();
    return m_matrices[0];
}

void MatrixStack::push( const Matrix4f& m )
{
	// Push m onto the stack.
	// Your stack should have OpenGL semantics:
	// the new top should be the old top multiplied by m
    m_matrices.push_back(m);
    m_matrices.insert(m_matrices.begin(), m_matrices[0] * m);
}

void MatrixStack::pop()
{
	// Remove the top element from the stack
    m_matrices.erase(m_matrices.begin());
}
