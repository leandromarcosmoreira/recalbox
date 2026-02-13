//
// Created by bkg2k on 15/06/25.
//
#pragma once

#include "GameType.h"

class FileData;

struct TypedFileData
{
  FileData* mData;
  GameType mType;
  // Allow lookuo on FileData, whatever the type is
  bool operator == (const TypedFileData& rhs) const { return mData == rhs.mData; }
  bool operator != (const TypedFileData& rhs) const { return mData != rhs.mData; }
  // Type identification
  bool IsChild() const { return mType == GameType::ChildGame || mType == GameType::ArcadeClone; }
};

