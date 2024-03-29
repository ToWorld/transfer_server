#ifndef MUDUO_BASE_COPYABLE_H__
#define MUDUO_BASE_COPYABLE_H__

/// A tag class emphasises the objects are copyable.
/// The empty base class optimization applies.
/// Any derived class of copyable should be a value type
class copyable {};

#endif // MUDUO_BASE_COPYABLE_H__
