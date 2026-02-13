//
// Created by gugue_U on 17/10/2020
//

#pragma once

#include <vector>
#include <random>
#include <games/FileData.h>
#include <systems/SystemManager.h>
#include <utils/datetime/HighResolutionTimer.h>
#include <components/TextComponent.h>
#include <components/VideoComponent.h>
#include <components/RatingComponent.h>
#include <components/ScrollableContainer.h>
#include <components/ClippingContainer.h>

#include <themes/ThemeExtras.h>
#include <components/DateTimeComponent.h>
#include "RecalboxConf.h"
#include "TextScrollComponent.h"

class GameClipContainer : public Gui
                        , public IThemeSwitchable
{

  private:

    WindowManager& mWindow;

    void initMDLabels();

    void initMDValues();

    std::vector<TextComponent*> getMDLabels();

    std::vector<ThemableComponent*> getMDValues();

    VideoComponent mVideo;
    ImageComponent mImage;
    ImageComponent mThumbnail;
    ImageComponent mRecalboxLogo;
    ClippingContainer mClippingContainer;
    ImageComponent mClippingImage;

    TextComponent mLbGameName, mLbSystemName, mLblRating, mLblReleaseDate, mLblDeveloper, mLblPublisher, mLblGenre, mLblPlayers, mLblLastPlayed, mLblPlayCount, mLblFavorite;
    TextScrollComponent mGameName;
    TextScrollComponent mSystemName;
    RatingComponent mRating;
    TextScrollComponent mReleaseDate;
    TextScrollComponent mDeveloper;
    TextScrollComponent mPublisher;
    TextScrollComponent mGenre;
    TextScrollComponent mPlayers;
    DateTimeComponent mLastPlayed;
    TextScrollComponent mPlayCount;
    TextScrollComponent mFavorite;
    ImageComponent mFavoriteIcon;

    ScrollableContainer mDescContainer;
    TextComponent mDescription;
    TextComponent mHeaderText;
    ImageComponent mHeaderImage;
    ImageComponent mBackground;

    ThemeExtras mThemeExtras;
    FileData* mGame;
    SystemData* mSystem;

    void initComponents();

    /*
     * IThemeSwitchable
     */

    /*!
     * @brief Gameclip view has a current game's system
     * @return SystemData
     */
    [[nodiscard]] SystemData* SystemTheme() const override { return mSystem; };

    /*!
     * @brief Called when theme switch.
     * @param theme New Theme
     * @param refreshOnly True if we need to refresh the current view only
     */
    void SwitchToTheme(const ThemeData& theme, bool refreshOnly, IThemeSwitchTick* interface) override;

  public:

    explicit GameClipContainer(WindowManager& window);

    void Render(const Transform4x4f& parentTrans) override;

    void setGameInfo(FileData* game);

    void CleanVideo();

    Vector2f getVideoCenter();

    bool CollectHelpItems(Help& help) override;

    ThemeExtras::List& Extras() { return mThemeExtras.Extras(); }
};