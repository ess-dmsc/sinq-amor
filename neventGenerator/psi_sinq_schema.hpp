// automatically generated by the FlatBuffers compiler, do not modify

#ifndef FLATBUFFERS_GENERATED_PSISINQSCHEMA_H_
#define FLATBUFFERS_GENERATED_PSISINQSCHEMA_H_

#include "flatbuffers/flatbuffers.h"

struct Event;

struct Event FLATBUFFERS_FINAL_CLASS : private flatbuffers::Table {
  enum {
    VT_HTYPE = 4,
    VT_TS = 6,
    VT_HWS = 8,
    VT_ST = 10,
    VT_PID = 12,
    VT_DATA = 14
  };
  const flatbuffers::String *htype() const { return GetPointer<const flatbuffers::String *>(VT_HTYPE); }
  uint64_t ts() const { return GetField<uint64_t>(VT_TS, 0); }
  const flatbuffers::Vector<uint16_t> *hws() const { return GetPointer<const flatbuffers::Vector<uint16_t> *>(VT_HWS); }
  uint64_t st() const { return GetField<uint64_t>(VT_ST, 0); }
  uint64_t pid() const { return GetField<uint64_t>(VT_PID, 0); }
  const flatbuffers::Vector<uint64_t> *data() const { return GetPointer<const flatbuffers::Vector<uint64_t> *>(VT_DATA); }
  bool Verify(flatbuffers::Verifier &verifier) const {
    return VerifyTableStart(verifier) &&
           VerifyField<flatbuffers::uoffset_t>(verifier, VT_HTYPE) &&
           verifier.Verify(htype()) &&
           VerifyField<uint64_t>(verifier, VT_TS) &&
           VerifyField<flatbuffers::uoffset_t>(verifier, VT_HWS) &&
           verifier.Verify(hws()) &&
           VerifyField<uint64_t>(verifier, VT_ST) &&
           VerifyField<uint64_t>(verifier, VT_PID) &&
           VerifyField<flatbuffers::uoffset_t>(verifier, VT_DATA) &&
           verifier.Verify(data()) &&
           verifier.EndTable();
  }
};

struct EventBuilder {
  flatbuffers::FlatBufferBuilder &fbb_;
  flatbuffers::uoffset_t start_;
  void add_htype(flatbuffers::Offset<flatbuffers::String> htype) { fbb_.AddOffset(Event::VT_HTYPE, htype); }
  void add_ts(uint64_t ts) { fbb_.AddElement<uint64_t>(Event::VT_TS, ts, 0); }
  void add_hws(flatbuffers::Offset<flatbuffers::Vector<uint16_t>> hws) { fbb_.AddOffset(Event::VT_HWS, hws); }
  void add_st(uint64_t st) { fbb_.AddElement<uint64_t>(Event::VT_ST, st, 0); }
  void add_pid(uint64_t pid) { fbb_.AddElement<uint64_t>(Event::VT_PID, pid, 0); }
  void add_data(flatbuffers::Offset<flatbuffers::Vector<uint64_t>> data) { fbb_.AddOffset(Event::VT_DATA, data); }
  EventBuilder(flatbuffers::FlatBufferBuilder &_fbb) : fbb_(_fbb) { start_ = fbb_.StartTable(); }
  EventBuilder &operator=(const EventBuilder &);
  flatbuffers::Offset<Event> Finish() {
    auto o = flatbuffers::Offset<Event>(fbb_.EndTable(start_, 6));
    return o;
  }
};

inline flatbuffers::Offset<Event> CreateEvent(flatbuffers::FlatBufferBuilder &_fbb,
    flatbuffers::Offset<flatbuffers::String> htype = 0,
    uint64_t ts = 0,
    flatbuffers::Offset<flatbuffers::Vector<uint16_t>> hws = 0,
    uint64_t st = 0,
    uint64_t pid = 0,
    flatbuffers::Offset<flatbuffers::Vector<uint64_t>> data = 0) {
  EventBuilder builder_(_fbb);
  builder_.add_pid(pid);
  builder_.add_st(st);
  builder_.add_ts(ts);
  builder_.add_data(data);
  builder_.add_hws(hws);
  builder_.add_htype(htype);
  return builder_.Finish();
}

inline flatbuffers::Offset<Event> CreateEvent(flatbuffers::FlatBufferBuilder &_fbb,
    const char *htype = nullptr,
    uint64_t ts = 0,
    const std::vector<uint16_t> *hws = nullptr,
    uint64_t st = 0,
    uint64_t pid = 0,
    const std::vector<uint64_t> *data = nullptr) {
  return CreateEvent(_fbb, htype ? 0 : _fbb.CreateString(htype), ts, hws ? 0 : _fbb.CreateVector<uint16_t>(*hws), st, pid, data ? 0 : _fbb.CreateVector<uint64_t>(*data));
}

inline const Event *GetEvent(const void *buf) { return flatbuffers::GetRoot<Event>(buf); }

inline bool VerifyEventBuffer(flatbuffers::Verifier &verifier) { return verifier.VerifyBuffer<Event>(nullptr); }

inline void FinishEventBuffer(flatbuffers::FlatBufferBuilder &fbb, flatbuffers::Offset<Event> root) { fbb.Finish(root); }

#endif  // FLATBUFFERS_GENERATED_PSISINQSCHEMA_H_
