#include "text_processing/suffix_array_builder.hpp"
#include "text_processing/naive_suffix_builder.hpp"

namespace text_processing {
    std::unique_ptr<SuffixArrayBuilder> SuffixArrayBuilder::create(BuilderType type) {
        switch (type) {
            case BuilderType::NAIVE:
                return std::make_unique<NaiveSuffixBuilder>();
            default:
                throw std::invalid_argument("Unknown builder type");
        }
    }
} // namespace text_processing