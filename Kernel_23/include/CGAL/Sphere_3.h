// Copyright (c) 1999  Utrecht University (The Netherlands),
// ETH Zurich (Switzerland), Freie Universitaet Berlin (Germany),
// INRIA Sophia-Antipolis (France), Martin-Luther-University Halle-Wittenberg
// (Germany), Max-Planck-Institute Saarbruecken (Germany), RISC Linz (Austria),
// and Tel-Aviv University (Israel).  All rights reserved.
//
// This file is part of CGAL (www.cgal.org); you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public License as
// published by the Free Software Foundation; version 2.1 of the License.
// See the file LICENSE.LGPL distributed with CGAL.
//
// Licensees holding a valid commercial license may use this file in
// accordance with the commercial license agreement provided with the software.
//
// This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
// WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
//
// $URL$
// $Id$
// 
//
// Author(s)     : Stefan Schirra

#ifndef CGAL_SPHERE_3_H
#define CGAL_SPHERE_3_H

CGAL_BEGIN_NAMESPACE

template <class R_>
class Sphere_3 : public R_::Kernel_base::Sphere_3
{
  typedef typename R_::FT                    FT;
  typedef typename R_::Point_3               Point_3;
  typedef typename R_::Aff_transformation_3  Aff_transformation_3;
  typedef typename R_::Kernel_base::Sphere_3  RSphere_3;
public:

  typedef RSphere_3 Rep;

  const Rep& rep() const
  {
    return *this;
  }

  Rep& rep()
  {
    return *this;
  }

  typedef          R_                       R;

      Sphere_3() {}

      Sphere_3(const RSphere_3& s)
      : RSphere_3(s) {}

      Sphere_3(const Point_3& p, const FT& sq_rad,
               const Orientation& o = COUNTERCLOCKWISE)
       : RSphere_3(p, sq_rad, o) {}

      Sphere_3(const Point_3& p, const Point_3& q,
               const Point_3& r, const Point_3& u)
       : RSphere_3(p, q, r, u) {}

      Sphere_3(const Point_3& p, const Point_3& q, const Point_3& r,
               const Orientation& o = COUNTERCLOCKWISE)
       : RSphere_3(p, q, r, o) {}

      Sphere_3(const Point_3& p, const Point_3&  q,
               const Orientation& o = COUNTERCLOCKWISE)
       : RSphere_3(p, q, o) {}

      Sphere_3(const Point_3& p, const Orientation& o = COUNTERCLOCKWISE)
       : RSphere_3(p, o) {}

    Sphere_3 orthogonal_transform(const Aff_transformation_3 &t) const;

    // FIXME : why doesn't Qrt work here ?  We loose optimization !
    //typename Qualified_result_of<typename R::Construct_center_3, Sphere_3>::type
    Point_3
    center() const
    {
      return R().construct_center_3_object()(*this);
    }

};

template <class R_>
Sphere_3<R_>
Sphere_3<R_>::
orthogonal_transform(const typename Sphere_3<R_>::Aff_transformation_3& t) const
{
    typedef typename  R_::RT  RT;
    typedef typename  R_::FT  FT;
    typedef typename  R_::Vector_3  Vector_3;

    // FIXME: precond: t.is_orthogonal() (*UNDEFINED*)
    Vector_3 vec(RT(1), RT(0), RT(0));        // unit vector
    vec = vec.transform(t);                   // transformed
    FT sq_scale = vec.squared_length();       // squared scaling factor

    return Sphere_3(t.transform(this->center()),
                    sq_scale * this->squared_radius(),
                    t.is_even() ? this->orientation()
                                : CGAL::opposite(this->orientation()));
}

CGAL_END_NAMESPACE

#endif // CGAL_SPHERE_3_H
