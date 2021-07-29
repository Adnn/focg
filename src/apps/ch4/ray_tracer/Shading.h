#pragma once


namespace ad {
namespace focg {


template <class T_forwardIterator>
math::hdr::Rgb shade(math::Position<3> aPoint, math::UnitVec<3> aNormal, const Material & aMaterial, 
                     math::Position<3> aViewPoint, 
                     const Scene & aScene,
                     math::hdr::Rgb aAmbientLight, T_forwardIterator aBeginLight, const T_forwardIterator aEndLight)
{
    // TODO this direction is actually minus ray direction
    math::UnitVec<3> viewDirection{aViewPoint - aPoint};
    math::hdr::Rgb color = aMaterial.ambientColor.cwMul(aAmbientLight);
    
    for(;
        aBeginLight != aEndLight;
        ++aBeginLight)
    {
        math::UnitVec<3> lightDirection{aBeginLight->position - aPoint};
        // Test if in shadow of current light
        if (! aScene.geometry->hit(Ray{aPoint, lightDirection}, Interval{Interval::gEpsilon}))
        {
            math::UnitVec<3> halfDirection{lightDirection + viewDirection};
            color +=  aBeginLight->intensity.cwMul(aMaterial.diffuseColor * std::max(0., aNormal.dot(lightDirection))
                                                   + aMaterial.specularColor * std::pow(std::max(0., aNormal.dot(halfDirection)),
                                                                                    aMaterial.phongExponent));
        }
    }

    return color;
}

} // namespace focg
} // namespace ad
