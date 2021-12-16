#pragma once

#include <arte/Image.h>

#include <math/Color.h>
#include <math/Rectangle.h>

#include <sstream>
#include <vector>


namespace ad {


class BayerPattern
{
public:
    struct Sample
    {
        std::size_t channel;

        // Each color [0..2] sampling position(s)
        std::array<std::vector<math::Vec<2, int>>, 3> demosaicSamples;
    };

    struct LinePattern
    {
        const Sample & getSample(std::size_t aColumn) const
        { return samples.at(aColumn % samples.size()); }

        std::vector<Sample> samples;
    };


    BayerPattern(std::vector<LinePattern> aLines) :
        mLines{std::move(aLines)}
    {}

    template <class T_sourcePixel>
    arte::Image<math::sdr::Grayscale> filter(const arte::Image<T_sourcePixel> & aSource) const;

    arte::Image<math::sdr::Rgb> colorCode(const arte::Image<math::sdr::Grayscale> & aFiltered) const;

    arte::Image<math::sdr::Rgb> demosaic(const arte::Image<math::sdr::Grayscale> & aFiltered) const;

private:
    std::size_t getChannelIndex(std::size_t aColumn, std::size_t aRow) const;

    const Sample & getSample(std::size_t aColumn, std::size_t aRow) const;

    math::sdr::Rgb reconstructPixel(std::size_t aColumn, std::size_t aRow,
                                    const arte::Image<math::sdr::Grayscale> & aFiltered) const;

    std::vector<LinePattern> mLines;
};


//
// Implementations
//

template <class T_sourcePixel>
arte::Image<math::sdr::Grayscale> BayerPattern::filter(const arte::Image<T_sourcePixel> & aSource) const
{
    arte::Image<math::sdr::Grayscale> result{aSource.dimensions(), {0}}; // black image
    for(int row = 0; row != aSource.height(); ++row)
    {
        for(int col = 0; col != aSource.width(); ++col)
        {
            result.at(col, row) = aSource.at(col, row).at(getChannelIndex(col, row));
        }
    }

    return result;
}


arte::Image<math::sdr::Rgb> BayerPattern::colorCode(const arte::Image<math::sdr::Grayscale> & aFiltered) const
{
    arte::Image<math::sdr::Rgb> result{aFiltered.dimensions(), math::sdr::gBlack}; // black image
    for(int row = 0; row != aFiltered.height(); ++row)
    {
        for(int col = 0; col != aFiltered.width(); ++col)
        {
            result.at(col, row).at(getChannelIndex(col, row)) = aFiltered.at(col, row);
        }
    }

    return result;
}


arte::Image<math::sdr::Rgb> BayerPattern::demosaic(const arte::Image<math::sdr::Grayscale> & aFiltered) const
{
    arte::Image<math::sdr::Rgb> result{aFiltered.dimensions(), math::sdr::gBlack}; // black image
    for(int row = 0; row != aFiltered.height(); ++row)
    {
        for(int col = 0; col != aFiltered.width(); ++col)
        {
            result.at(col, row) = reconstructPixel(col, row, aFiltered);
        }
    }

    return result;
}


inline const BayerPattern::Sample & BayerPattern::getSample(std::size_t aColumn, std::size_t aRow) const
{
    return mLines.at(aRow % mLines.size()).getSample(aColumn);
}


inline std::size_t BayerPattern::getChannelIndex(std::size_t aColumn, std::size_t aRow) const
{
    return getSample(aColumn, aRow).channel;
}


math::sdr::Rgb BayerPattern::reconstructPixel(std::size_t aColumn, std::size_t aRow,
                                              const arte::Image<math::sdr::Grayscale> & aFiltered) const
{
    math::sdr::Rgb result = math::sdr::gBlack;
    const math::Position<2, int> basePos{(int)aColumn, (int)aRow};
    const math::Rectangle<int> imageRect{ {0, 0}, aFiltered.dimensions() };

    for (int channel = 0; channel != 3; ++channel)
    {
        int validSamples = 0;
        int sum = 0;

        for (auto offset : getSample(aColumn, aRow).demosaicSamples[channel])
        {
            auto samplePos = basePos + offset;
            if (imageRect.contains(samplePos))
            {
                ++validSamples;
                sum += aFiltered.at(samplePos);
            }
        }

        if (validSamples == 0)
        {
            std::ostringstream out;
            out << "No valid samples at position " << basePos << " for channel " << channel;
            throw std::logic_error{out.str()};
        }

        result[channel] = sum/validSamples;
    }

    return result;
}


} // namespace ad
