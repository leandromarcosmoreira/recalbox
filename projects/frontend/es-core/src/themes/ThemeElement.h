//
// Created by bkg2k on 09/11/2019.
//
#pragma once

#include <map>
#include <utils/String.h>
#include <utils/os/fs/Path.h>
#include <utils/math/Vector2f.h>
#include "ElementTypes.h"
#include "PropertyNames.h"
#include "utils/storage/HashMap.h"
#include "rendering/fonts/Alignment.h"
#include "ThemeContexts.h"
#include "IThemeContextualVariableResolver.h"
#include "ThemeVariablesHolder.h"
#include "games/ItemType.h"

class Renderer;

class ThemeElement
{
  public:
    static bool ConvertPair(const String& string, float& x, float& y);

  private:
    //! Property type
    enum class Type: char
    {
      Vector,     //!< Stored as vector
      String,     //!< Stored as string
      Integer,    //!< Stored as integer
      Float,      //!< Stored as single floating value
      Bool,       //!< Stored as boolean
      Unresolved, //!< Stored as raw string w/ unresolved variables
    };
    //! Bag of data
    class PropertyBag
    {
      private:
        String mString;
        union // Save some space by union-ing cardinal exclusive values
        {
          int mInteger;
          float mFloat;
          bool mBool;
        };
        float mSecondFloat;
        Type mType;
        ThemeContexts mContext;

      public:
        PropertyBag()                              : mInteger(0), mSecondFloat(0.0f), mType(Type::String), mContext(ThemeContexts::None) {}
        explicit PropertyBag(const Vector2f& v)    : mFloat(v.x()), mSecondFloat(v.y()), mType(Type::Vector), mContext(ThemeContexts::None) {}
        explicit PropertyBag(float v1, float v2)   : mFloat(v1), mSecondFloat(v2), mType(Type::Vector), mContext(ThemeContexts::None) {}
        explicit PropertyBag(const String& s)      : mString(s), mInteger(0), mSecondFloat(0.0f), mType(Type::String), mContext(ThemeContexts::None) {}
        explicit PropertyBag(int v)                : mInteger(v), mSecondFloat(0.0f), mType(Type::Integer), mContext(ThemeContexts::None) {}
        explicit PropertyBag(float v)              : mFloat(v), mSecondFloat(0.0f), mType(Type::Float), mContext(ThemeContexts::None) {}
        explicit PropertyBag(bool v)               : mBool(v), mSecondFloat(0.0f), mType(Type::Bool), mContext(ThemeContexts::None) {}
        explicit PropertyBag(const String& s, ThemeContexts context) : mString(s), mInteger(0), mSecondFloat(0.0f), mType(Type::Unresolved), mContext(context) {}

        [[nodiscard]] String AsString(ThemeVariablesHolder& variables) const
        {
          switch(mType)
          {
            case Type::Vector: return String(mFloat, 4).Append(' ').Append(mSecondFloat, 4);
            case Type::String: return mString;
            case Type::Integer: return String(mInteger);
            case Type::Float: return String(mFloat, 4);
            case Type::Bool: return String(mBool ? '1' : '0');
            case Type::Unresolved: String s(mString); variables.Resolve(s, mContext); return s;
          }
          return String();
        }
        [[nodiscard]] Path AsPath(ThemeVariablesHolder& variables) const
        {
          switch(mType)
          {
            case Type::Vector: return Path(String(mFloat, 4).Append(' ').Append(mSecondFloat, 4));
            case Type::String: return Path(mString).ToCanonical();
            case Type::Integer: return Path(String(mInteger));
            case Type::Float: return Path(String(mFloat, 4));
            case Type::Bool: return Path(String(mBool ? '1' : '0'));
            case Type::Unresolved: String s(mString); variables.Resolve(s, mContext); return Path(s);
          }
          return Path();
        }
        [[nodiscard]] int AsInt(ThemeVariablesHolder& variables) const
        {
          switch(mType)
          {
            case Type::String: { int result = 0; return mString.TryAsInt(result) ? result : 0; }
            case Type::Integer: return mInteger;
            case Type::Vector:
            case Type::Float: return (int)mFloat;
            case Type::Bool: return (int)mBool;
            case Type::Unresolved: { String s(mString); variables.Resolve(s, mContext); int result = 0; return mString.TryAsInt(result) ? result : 0; }
          }
          return 0;
        }
        [[nodiscard]] float AsFloat(ThemeVariablesHolder& variables) const
        {
          switch(mType)
          {
            case Type::String: { float result = 0; return mString.TryAsFloat(result) ? result : 0.0f; }
            case Type::Integer: return (float)mInteger;
            case Type::Vector:
            case Type::Float: return mFloat;
            case Type::Bool: return (float)mBool;
            case Type::Unresolved: { String s(mString); variables.Resolve(s, mContext); float result = 0; return mString.TryAsFloat(result) ? result : 0; }
          }
          return 0.0f;
        }
        [[nodiscard]] bool AsBool(ThemeVariablesHolder& variables) const
        {
          switch(mType)
          {
            case Type::String: { bool result = false; if (mString.TryAsBool(result)) return result; return false; }
            case Type::Integer: return (bool)mInteger;
            case Type::Vector:
            case Type::Float: return (bool)mFloat;
            case Type::Bool: return mBool;
            case Type::Unresolved: { String s(mString); variables.Resolve(s, mContext); bool result = false; return mString.TryAsBool(result) ? result : false; }
          }
          return false;
        }
        [[nodiscard]] Vector2f AsVector(ThemeVariablesHolder& variables) const
        {
          switch(mType)
          {
            case Type::Vector: return { mFloat, mSecondFloat };
            case Type::String:
            {
              float x = 0;
              float y = 0;
              if (ConvertPair(mString, x, y))
                return { x, y };
              break;
            }
            case Type::Integer: return { (float)mInteger, (float)mInteger };
            case Type::Float: return { mFloat, mFloat };
            case Type::Bool: return {(float)mBool, (float)mBool};
            case Type::Unresolved:
            {
              String s(mString); variables.Resolve(s, mContext);
              float x = 0;
              float y = 0;
              if (s.TryAsFloat(0, ' ', x))
              {
                int pos = s.Find(' ');
                if (pos >= 0)
                  if (s.TryAsFloat(pos + 1, 0, y))
                    return { x, y };
              }
              break;
            }
          }
          return { 0.0f, 0.0f };
        }
    };

    //! Properties
    HashMap<ThemePropertyName, PropertyBag> mProperties;
    //! Property map
    ThemePropertyNameBits mAvailableProperties;
    //! Property map
    ThemePropertyNameBits mLocalizedProperties;
    //! Element name
    String mName;
    //! Variable resolver
    ThemeVariablesHolder& mVariables;
    //! Element type
    ThemeElementType mType;
    //! Is extra element
    bool mExtra;
    //! Merged property contexts
    ThemeContexts mMergedContext;
    //! Limited view
    ThemeItemType mLimitedVisibility;

  public:
    /*!
     * @brief Constructor
     * @param name Element name
     * @param type Element type
     * @param extra Element is an extra
     */
    ThemeElement(const String& name, ThemeElementType type, bool extra, ThemeItemType limitedVisibility, ThemeVariablesHolder& variables)
      : mName(name)
      , mVariables(variables)
      , mType(type)
      , mExtra(extra)
      , mMergedContext(ThemeContexts::None)
      , mLimitedVisibility(limitedVisibility)
    {
    }

    [[nodiscard]] String AsString(ThemePropertyName name) const
    {
      PropertyBag* bag = mProperties.try_get(name);
      return bag != nullptr ? bag->AsString(mVariables) : String::Empty;
    }

    [[nodiscard]] Path AsPath(ThemePropertyName name) const
    {
      PropertyBag* bag = mProperties.try_get(name);
      return bag != nullptr ? bag->AsPath(mVariables) : Path::Empty;
    }

    [[nodiscard]] int AsInt(ThemePropertyName name) const
    {
      PropertyBag* bag = mProperties.try_get(name);
      return bag != nullptr ? bag->AsInt(mVariables) : 0;
    }

    [[nodiscard]] float AsFloat(ThemePropertyName name) const
    {
      PropertyBag* bag = mProperties.try_get(name);
      return bag != nullptr ? bag->AsFloat(mVariables) : 0.f;
    }

    [[nodiscard]] bool AsBool(ThemePropertyName name) const
    {
      PropertyBag* bag = mProperties.try_get(name);
      return bag != nullptr && bag->AsBool(mVariables);
    }

    [[nodiscard]] Vector2f AsVector(ThemePropertyName name) const
    {
      PropertyBag* bag = mProperties.try_get(name);
      return bag != nullptr ? bag->AsVector(mVariables) : Vector2f(0.0f, 0.0f);
    }

    [[nodiscard]] Alignment AsAlignment(ThemePropertyName name) const
    {
      static HashMap<String, Alignment> sStringToAlignment
      {
        { "left"         , Alignment::CenterLeft },
        { "right"        , Alignment::CenterRight },
        { "top"          , Alignment::TopCenter },
        { "bottom"       , Alignment::BottomCenter },
        { "topleft"      , Alignment::TopLeft },
        { "topcenter"    , Alignment::TopCenter },
        { "topright"     , Alignment::TopRight },
        { "centerleft"   , Alignment::CenterLeft },
        { "center"       , Alignment::Center },
        { "centerright"  , Alignment::CenterRight },
        { "bottomleft"   , Alignment::BottomLeft },
        { "bottomcenter" , Alignment::BottomCenter },
        { "bottomright"  , Alignment::BottomRight },
      };
      PropertyBag* bag = mProperties.try_get(name);
      if (bag == nullptr) return Alignment::CenterLeft;
      Alignment* result = sStringToAlignment.try_get(bag->AsString(mVariables).LowerCase());
      return result != nullptr ? *result : Alignment::CenterLeft;
    }

    [[nodiscard]] bool HasProperty(ThemePropertyName property) const { return mAvailableProperties.IsSet(property); }

    [[nodiscard]] bool HasProperties() const { return !mProperties.empty(); }

    [[nodiscard]] const String& Name() const { return mName; }

    [[nodiscard]] ThemeElementType Type() const { return mType; }

    [[nodiscard]] bool Extra() const { return mExtra; }

    [[nodiscard]] ThemeItemType LimitedVisibility() const { return mLimitedVisibility; }

    [[nodiscard]] bool HasUnresolvedVariables() const { return !isEmpty(mMergedContext); }

    [[nodiscard]] bool HasLimitedVisiblity() const { return !isEmpty(mLimitedVisibility); }

    void AddVectorProperty(ThemePropertyName name, float x, float y) { mProperties[name] = PropertyBag(x, y); mAvailableProperties.Set(name); }
    void AddVectorProperty(ThemePropertyName name, const Vector2f& v) { mProperties[name] = PropertyBag(v); mAvailableProperties.Set(name); }
    void AddStringProperty(ThemePropertyName name, const String& s) { mProperties[name] = PropertyBag(s); mAvailableProperties.Set(name); }
    void AddVariableProperty(ThemePropertyName name, const String& s, ThemeContexts context) { mProperties[name] = PropertyBag(s, context); mAvailableProperties.Set(name); mMergedContext |= context; }
    void AddIntProperty(ThemePropertyName name, int v) { mProperties[name] = PropertyBag(v); mAvailableProperties.Set(name); }
    void AddFloatProperty(ThemePropertyName name, float v) { mProperties[name] = PropertyBag(v); mAvailableProperties.Set(name); }
    void AddBoolProperty(ThemePropertyName name, bool v) { mProperties[name] = PropertyBag(v); mAvailableProperties.Set(name); }

    ThemeElement& MergeExtra(bool extra) { mExtra |= extra; return *this; }
    ThemeElement& MergeVisibility(ThemeItemType limitedVisibility) { mLimitedVisibility |= limitedVisibility; return *this; }

    [[nodiscard]] bool IsAlreadyLocalized(ThemePropertyName property) const { return mLocalizedProperties.IsSet(property); }
    void SetLocalized(ThemePropertyName property) { mLocalizedProperties.Set(property); }
};
