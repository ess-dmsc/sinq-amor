#pragma once

namespace SINQAmorSim {

template <typename T> struct StreamFormat { using value_type = T; };
using PSIformat = StreamFormat<uint64_t>;
using ESSformat = StreamFormat<uint32_t>;

}
