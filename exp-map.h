/* exp-map.h
 * 
 * Definitions and datatypes necessary for computing rotations with
 * the exponential map parameterization.  This package provides
 * functions for computing a rotation matrix, its partial derivatives
 * with respect to its exponential map parameters, and the angular
 * velocity update necessary for dynamic simulation for both the three
 * and two DOF rotations.  These functions do not constitute an
 * efficient implementation, since there is no provision for caching
 * the intermediate quaternion and trig quantities, because the form
 * of caching will depend on the modularization strategy (C++ object,
 * etc.)  The derivative calculations, in particular, should not be
 * benchmarked without a supplemental caching scheme.
 * 
 * Please see the copyright notice in exp-map.c
 *
 * Copyright (C) 1997    F. Sebastian Grassia
 */

#pragma once

typedef double ExpMapType;

typedef ExpMapType Quat[4];

/* Compute rotation matrix from 3 or 2 DOF exponential map (EM) vector */
void EM3_To_R(ExpMapType v[3], ExpMapType R[4][4]);
void EM2_To_R(ExpMapType r[2], ExpMapType s[3], ExpMapType t[3], ExpMapType R[4][4]);
int EM_To_Q(ExpMapType v[3], Quat q, int reparam);

/* Compute i'th partial derivative of rotation matrix wrt 3 or
 * 2 DOF EM vector */
int Partial_R_Partial_EM3(ExpMapType v[3], int i, ExpMapType dRdvi[4][4]);
int Partial_R_Partial_EM2(ExpMapType r[2], ExpMapType s[3], ExpMapType t[3],
			  int i, ExpMapType dRdvi[4][4]);

/* Compute time deriv of 'v' given angular velocity 'omega' */
void Vdot(ExpMapType v[3], ExpMapType omega[3], ExpMapType vdot[3]);
