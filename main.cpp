#include <cocos2d.h>
#include <include/gd.h>
#include <string>
#include <fstream>
#include "sai/hook.h"
#include "sai/MensajeDesaparece.h"
#include <unordered_map>

int m_nMainTag = 7777;
int m_nNameTag = 7778;
float modVersion = 1.3f;

#define glm gd::GameLevelManager::sharedState()
#define gm gd::GameManager::sharedState()

bool isSurroundedByNonLetters(const std::string& text, size_t pos, size_t wordLength) {
    bool isStartSurrounded = (pos == 0 || !std::isalpha(text[pos - 1]));
    bool isEndSurrounded = (pos + wordLength == text.length() || !std::isalpha(text[pos + wordLength]));
    return isStartSurrounded && isEndSurrounded;
}

bool compareByLengthDesc(const std::string& a, const std::string& b) {
    return a.length() > b.length();
}

bool isNumber(const std::string& str) {
    for (char c : str)
        if (isdigit(c)) return true;
        else return false;
}

std::unordered_map<std::string, std::string> translationMap;
bool cargarUnaVez = true;

typedef void* (__fastcall* _SetString)(CCLabelBMFont*, void*, const char*, bool);
_SetString SetString;

void* __fastcall hookSetString(CCLabelBMFont* self, void* edx, const char* str, bool needUpdateLabel) {
    std::string inputText = str;

    if (!gm->getGameVariable("TraducirGD")) {
        if (inputText.size() > 0 && !isNumber(inputText) && self->getTag() != m_nMainTag && (self->getTag() != m_nNameTag || gm->getGameVariable("TraducirNiveles"))) {
            if (cargarUnaVez) {
                std::ifstream inputFile("traduccion.txt");
                std::string line;

                if (!inputFile) inputFile.open("Resources/traduccion.txt", std::ios::in);

                while (std::getline(inputFile, line)) {
                    size_t delimiterPos = line.find(',');
                    if (delimiterPos != std::string::npos) {
                        size_t startPos = line.find("\"");
                        size_t endPos = line.find("\"", startPos + 1);
                        if (startPos != std::string::npos && endPos != std::string::npos) {
                            std::string findText = line.substr(startPos + 1, endPos - startPos - 1);
                            std::replace(findText.begin(), findText.end(), '\\', '\n');

                            startPos = line.find("\"", endPos + 1);
                            endPos = line.find("\"", startPos + 1);
                            if (startPos != std::string::npos && endPos != std::string::npos) {
                                std::string replaceText = line.substr(startPos + 1, endPos - startPos - 1);
                                std::replace(replaceText.begin(), replaceText.end(), '\\', '\n');

                                translationMap[findText] = replaceText;
                            }
                        }
                    }
                }

                cargarUnaVez = false;
            }

            std::vector<std::string> replaceTerms;
            for (const auto& rule : translationMap) 
                replaceTerms.push_back(rule.first);
            
            std::sort(replaceTerms.begin(), replaceTerms.end(), compareByLengthDesc);

            for (const auto& term : replaceTerms) {
                size_t pos = inputText.find(term);
                while (pos != std::string::npos) {
                    if (isSurroundedByNonLetters(inputText, pos, term.length()))
                        inputText.replace(pos, term.length(), translationMap[term]);

                    pos = inputText.find(term, pos + translationMap[term].length());
                }
            }
        }
    }
    return SetString(self, edx, inputText.c_str(), needUpdateLabel);;
}

#pragma warning(disable : 4996)
time_t GD22 = 1697968800; // 22-10-2023 | 10:00:00

class Temporizador : public CCLayer {
    CCLabelBMFont* fecha;
    extension::CCScale9Sprite* fondo;
public:
    bool init() {
        auto size = CCDirector::sharedDirector()->getWinSize();

        auto menu = CCMenu::create(importante, nullptr);
        menu->setPosition({ size.width - 50, size.height / 2 + 65 });
        this->addChild(menu);

        fondo = extension::CCScale9Sprite::create("square02_small.png");
        fondo->setOpacity(125);
        fondo->setPosition({ size.width - 50, 115 });
        fondo->setContentSize({ 150, 70 });

        auto titulo = CCLabelBMFont::create("Version 2.2", "goldFont.fnt");
        titulo->setPosition({ fondo->getScaledContentSizeX() / 2, fondo->getScaledContentSizeY() / 2 + 18 });
        titulo->limitLabelWidth(fondo->getScaledContentSizeX() - 20, 1.0f, 0);
        fondo->addChild(titulo);

        fecha = CCLabelBMFont::create("", "bigFont.fnt");
        fecha->setPosition({ fondo->getScaledContentSizeX() / 2, titulo->getPositionY() - 30 });
        fondo->addChild(fecha);

        fondo->setScale(.55f);
        this->addChild(fondo);

        this->scheduleUpdate();
        return true;
    }
    void update(float dt) {
        std::tm* timeinfo = std::gmtime(&GD22);

        int timeLeft = GD22 - time(0);

        int days = timeLeft / (24 * 60 * 60);
        int hours = (timeLeft % (24 * 60 * 60)) / (60 * 60);
        int minutes = (timeLeft % (60 * 60)) / 60;
        int seconds = timeLeft % 60;

        if (seconds < 0 || minutes < 0 || hours < 0 || days < 0) {
            fecha->setString("Actualiza GD!");
        }
        else {
            fecha->setString(CCString::createWithFormat("%02d:%02d:%02d:%02d", days, hours, minutes, seconds)->getCString());
        }

        fecha->limitLabelWidth(fondo->getContentSizeX() - 20, .7f, 0);
    }
    CREATE_FUNC(Temporizador)
};

class PlayerInfoObject : public CCObject {
protected:
    string m_sUsername;
    int m_nIconID;
    int m_nColor1;
    int m_nColor2;
    int m_nUserID;
    int m_nAccountID;
    bool m_bGlow;
public:
    const char* getUsername() {
        return m_sUsername.c_str();
    }
    int getIcon() {
        return m_nIconID;
    }
    int getColor1() {
        return m_nColor1;
    }
    int getColor2() {
        return m_nColor2;
    }
    int getUserID() {
        return m_nUserID;
    }
    int getAccountID() {
        return m_nAccountID;
    }
    bool getGlow() {
        return m_bGlow;
    }
    bool init(string username, int icon, int col1, int col2, int userID, int accountID, bool glow = true) {
        m_sUsername = username;
        m_nIconID = icon;
        m_nColor1 = col1;
        m_nColor2 = col2;
        m_nUserID = userID;
        m_nAccountID = accountID;
        m_bGlow = glow;
        return true;
    }
    static PlayerInfoObject* create(string username, int icon, int col1, int col2, int userID, int accountID, bool glow = true) {
        auto r = new PlayerInfoObject;
        if (r && r->init(username, icon, col1, col2, userID, accountID, glow)) r->autorelease();
        else CC_SAFE_DELETE(r);
        return r;
    }
};

class Creditos : public gd::FLAlertLayer {
    extension::CCScale9Sprite* bg;
    CCLabelBMFont* page;
    gd::CCMenuItemSpriteExtra* arrowBtn;
    gd::CCMenuItemSpriteExtra* arrowBtn2;
    CCLayer* allPages;
    int pagina = 0;
    int totalPaginas = 0;
    CCLabelBMFont* specialP;
public:
    bool init() {
        if (initWithColor({ 0, 0, 0, 150 }));

        m_pLayer = CCLayer::create();
        m_pButtonMenu = CCMenu::create();
        m_pButtonMenu->setPosition(0, 0);

        auto dir = CCDirector::sharedDirector();

        dir->getTouchDispatcher()->incrementForcePrio(2);
        dir->getTouchDispatcher()->addTargetedDelegate(this, -1, true);

        this->setKeypadEnabled(true);

        auto size = dir->getWinSize();

        bg = extension::CCScale9Sprite::create("GJ_square01.png");
        bg->setContentSize({ 420, 280 });
        bg->setPosition({ size.width / 2, size.height / 2 });
        m_pLayer->addChild(bg);

        for (int i = 0; i < 4; i++) {
            auto pos = CCPoint({ bg->getPositionX() - bg->getContentSizeX() / 2 + 25, bg->getPositionY() - bg->getContentSizeY() / 2 + 25 });
            auto sideArt = CCSprite::createWithSpriteFrameName("dailyLevelCorner_001.png");

            switch (i) {
            case 1:
                sideArt->setFlipX(true);
                pos = CCPoint({ bg->getPositionX() + bg->getContentSizeX() / 2 - 25, bg->getPositionY() - bg->getContentSizeY() / 2 + 25 });
                break;
            case 2:
                sideArt->setFlipY(true);
                pos = CCPoint({ bg->getPositionX() - bg->getContentSizeX() / 2 + 25, bg->getPositionY() + bg->getContentSizeY() / 2 - 25 });
                break;
            case 3:
                sideArt->setFlipX(true);
                sideArt->setFlipY(true);
                pos = CCPoint({ bg->getPositionX() + bg->getContentSizeX() / 2 - 25, bg->getPositionY() + bg->getContentSizeY() / 2 - 25 });
                break;
            }

            sideArt->setPosition(pos);
            m_pLayer->addChild(sideArt);
        }

        auto salir = gd::CCMenuItemSpriteExtra::create(CCSprite::createWithSpriteFrameName("GJ_closeBtn_001.png"), this, menu_selector(Creditos::onSalir));
        salir->setPosition({ bg->getPositionX() - bg->getContentSizeX() / 2 + 10, bg->getPositionY() + bg->getContentSizeY() / 2 - 10 });
        m_pButtonMenu->addChild(salir);

        auto titulo = CCLabelBMFont::create(CCString::createWithFormat("GD Traducido v%.01f (Final)", modVersion)->getCString(), "bigFont.fnt");
        titulo->setScale(.7f);
        titulo->setPosition({ bg->getPositionX(), bg->getPositionY() + bg->getContentSizeY() / 2 - 18 });
        m_pLayer->addChild(titulo);

        specialP = CCLabelBMFont::create("", "bigFont.fnt");
        specialP->setAlignment(kCCTextAlignmentCenter);
        specialP->setColor({ 0, 255, 0 });
        specialP->setPosition({ bg->getPositionX(), bg->getPositionY() + 85 });
        m_pLayer->addChild(specialP);

        page = CCLabelBMFont::create("", "goldFont.fnt");
        page->setScale(.5f);
        page->setPosition({ bg->getPositionX() * 2 - 3, bg->getPositionY() * 2 - 3 });
        page->setAnchorPoint({ 1.0f, 1.0f });
        m_pLayer->addChild(page);

        auto arrow = CCSprite::createWithSpriteFrameName("GJ_arrow_01_001.png");
        arrow->setFlipX(true);
        arrowBtn = gd::CCMenuItemSpriteExtra::create(arrow, this, menu_selector(Creditos::onNext));
        arrowBtn->setPosition({ bg->getPositionX() * 2 - 20, bg->getPositionY() });
        arrowBtn->setVisible(false);
        m_pButtonMenu->addChild(arrowBtn);

        arrowBtn2 = gd::CCMenuItemSpriteExtra::create(CCSprite::createWithSpriteFrameName("GJ_arrow_01_001.png"), this, menu_selector(Creditos::onBack));
        arrowBtn2->setPosition({ 20, bg->getPositionY() });
        arrowBtn2->setVisible(false);
        m_pButtonMenu->addChild(arrowBtn2);

        allPages = CCLayer::create();
        this->generateIcons(0, 1);
        this->generateIcons(1, 14);
        this->generateIcons(15, 11);
        this->generateIcons(1000, am->m_nPlayerAccountID > 0 ? 2 : 1);

        if (totalPaginas > 1) {
            auto text = CCLabelBMFont::create("(Toca el perfil que quieras ver)", "chatFont.fnt");
            text->setColor(ccCELESTE);
            text->setScale(.7f);
            text->setPosition({ bg->getPositionX(), bg->getPositionY() - bg->getContentSizeY() / 2 + 25 });
            m_pLayer->addChild(text);
        }

        this->updatePage();

        m_pLayer->addChild(allPages);
        m_pLayer->addChild(m_pButtonMenu);
        this->addChild(m_pLayer);
        return true;
    }
    void onNext(CCObject*) {
        pagina++;
        if (pagina > totalPaginas - 1) pagina = totalPaginas - 1;

        if (auto layer = reinterpret_cast<CCLayer*>(allPages->getChildren()->objectAtIndex(pagina - 1))) layer->setVisible(false);

        this->updatePage();
    }
    void onBack(CCObject*) {
        pagina--;
        if (pagina < 0) pagina = 0;

        if (auto layer = reinterpret_cast<CCLayer*>(allPages->getChildren()->objectAtIndex(pagina + 1))) layer->setVisible(false);

        this->updatePage();
    }
    void updatePage() {
        if (totalPaginas > 1)
            page->setString(CCString::createWithFormat("Pagina %i de %i", pagina + 1, totalPaginas)->getCString());

        if (pagina <= 0) arrowBtn2->setVisible(false);
        else arrowBtn2->setVisible(true);

        if (pagina >= (totalPaginas - 1)) {
            arrowBtn->setVisible(false);
            specialP->setString("Por ultimo muchas gracias a Robtop\npor crear este maravilloso juego...\ny gracias a ti tambien :)");
        }
        else {
            arrowBtn->setVisible(true);
            switch (pagina) {
            case 0:
                specialP->setString("Mod creado por elSai");
                break;
            default:
                specialP->setString("Gente que ayudo en la traduccion\ndel juego");
                break;
            }
        }
        
        specialP->limitLabelWidth(300, .7f, 0);

        if (auto layer = reinterpret_cast<CCLayer*>(allPages->getChildren()->objectAtIndex(pagina))) layer->setVisible(true);
    }
    void generateIcons(int start, int iconsCount) {
        auto layer = CCLayer::create();
        layer->setVisible(false);

        if (start == 1000 || start == 0) {
            layer->setPositionY(-50);
            layer->setScale(1.5f);
        }

        auto menu = CCMenu::create();
        menu->setPosition(0, 0);

        float padX = 0;
        float padY = 0;
        int maxItemsPerRow = 7;

        int totalItems = iconsCount;

        while (totalItems > 0) {
            int itemsInCurrentRow = min(maxItemsPerRow, totalItems);
            float rowOffsetX = (itemsInCurrentRow - 1) * (170 * 2) / (6 * 2);
            float offsetX = padX - rowOffsetX;

            for (int i = 1; i <= itemsInCurrentRow; i++) {
                int currentItemIndex = start + i + (iconsCount - totalItems);
                this->createIconSetup(currentItemIndex, layer, menu, offsetX, padY);
                offsetX += (170 * 2) / 6;
            }

            totalItems -= itemsInCurrentRow;
            padX = ((170 * 2) / 6) * (maxItemsPerRow - itemsInCurrentRow) / 2.0f;
            padY += 80;
        }

        layer->addChild(menu);

        allPages->addChild(layer);
        totalPaginas++;
    }
    void onSalir(CCObject*) {
        CCDirector::sharedDirector()->getTouchDispatcher()->decrementForcePrio(2);
        CCDirector::sharedDirector()->getTouchDispatcher()->removeDelegate(this);
        this->removeFromParentAndCleanup(true);
    }
    void show() {
        m_pLayer->setPosition(0, 160);
        m_pLayer->runAction(CCEaseElasticOut::create(CCMoveTo::create(0.5f, { 0, 0 }), 0.6f));

        auto opacity = this->getOpacity();
        this->setOpacity(0);

        this->runAction(CCFadeTo::create(0.14f, opacity));
        CCDirector::sharedDirector()->getRunningScene()->addChild(this, CCDirector::sharedDirector()->getRunningScene()->getHighestChildZ());
    }
    void keyBackClicked() {
        this->onSalir(nullptr);
    }
    void createIconSetup(int id, CCLayer* layer, CCMenu* menu, float offsetX, float padY) {
        auto player = PlayerInfoObject::create("elSai", 110, 8, 40, 15797403, 5018589);

        switch (id) {
        case 2:
            player->init("SpaceDream", 79, 3, 7, 59222336, 8046029);
            break;
        case 3:
            player->init("leonardgamer7", 46, 15, 15, 148955706, 14627225);
            break;
        case 4:
            player->init("RovisRon", 59, 17, 11, 3918331, 792514);
            break;
        case 5:
            player->init("Gxterne", 65, 18, 17, 77818990, 8561191, false);
            break;
        case 6:
            player->init("AgenteDylkick", 35, 15, 10, 122089703, 12039492);
            break;
        case 7:
            player->init("zuramaruxd", 25, 18, 17, 109583843, 10878795, false);
            break;
        case 8:
            player->init("Virtualoid", 131, 18, 3, 18926316, 6440001);
            break;
        case 9:
            player->init("AshleyBitrate", 85, 6, 16, 195642392, 21746023, false);
            break;
        case 10:
            player->init("ItsDPR", 103, 18, 41, 141566799, 14207856);
            break;
        case 11:
            player->init("JacknMax360", 1, 2, 3, 5337042, 289369);
            break;
        case 12:
            player->init("MKAMV", 46, 22, 12, 15147534, 4677343);
            break;
        case 13:
            player->init("ImYoKiLoki", 113, 11, 19, 138448991, 14030187);
            break;
        case 14:
            player->init("Lucasberry", 102, 3, 12, 4787852, 183614);
            break;
        case 15:
            player->init("Abuigsito", 50, 3, 2, 146575363, 17058646);
            break;
        case 16:
            player->init("ImaginaryOrlan", 134, 12, 17, 63766876, 8445162, false);
            break;
        case 17:
            player->init("Nonixgames", 98, 3, 34, 120843273, 11959019, false);
            break; 
        case 18:
            player->init("DavideGamer", 49, 9, 12, 40657999, 6994898);
            break;
        case 19:
            player->init("GabRed01", 18, 9, 12, 4681878, 948533);
            break;
        case 20:
            player->init("allcraft555", 111, 37, 12, 42836652, 7283691);
            break;
        case 21:
            player->init("LeroZ", 37, 3, 18, 134290787, 13767957, false);
            break;
        case 22:
            player->init("Srluipech", 97, 3, 3, 171878632, 19674814);
            break;
        case 23:
            player->init("zUsdf123GD", 133, 18, 3, 9980667, 1760809);
            break;
        case 24:
            player->init("Tojamuz115", 127, 18, 17, 21134555, 6050816);
            break;
        case 25:
            player->init("hexagonforcev7", 0, 0, 3, 0, 0, false);
            break;
        case 26:
            player->init("CafecitoMoon", 109, 6, 3, 0, 0);
            break;
        case 1001:
            player->init("RobTop", 4, 14, 3, 16, 71);
            break;
        case 1002:
            player->init(am->getUsername(), gm->getPlayerFrame(), gm->getPlayerColor(), gm->getPlayerColor2(), 0, am->m_nPlayerAccountID, gm->getPlayerGlow());
            break;
        }

        auto fondo = extension::CCScale9Sprite::create("square02_small.png");
        fondo->setOpacity(id % 2 == 0 ? 30 : 40);
        fondo->setContentSize({ 56, 70 });

        auto nombre = CCLabelBMFont::create("", "goldFont.fnt");
        nombre->setTag(m_nMainTag);
        nombre->setString(player->getUsername());
        nombre->limitLabelWidth(50, 0.6, 0);
        nombre->setPosition({ fondo->getContentSizeX() / 2, fondo->getContentSizeY() - 8 });
        fondo->addChild(nombre);

        auto shadow = CCSprite::createWithSpriteFrameName("chest_shadow_001.png");
        shadow->setPosition({ nombre->getPositionX(), nombre->getPositionY() - 40 - 22 + 7 });
        shadow->setScale(.4f);
        shadow->setOpacity(45);
        fondo->addChild(shadow);

        auto i = gd::SimplePlayer::create(player->getIcon());
        i->setColor(gm->colorForIdx(player->getColor1()));
        i->setSecondColor(gm->colorForIdx(player->getColor2()));
        i->setGlowOutline(player->getGlow());
        i->setPosition({ nombre->getPositionX(), nombre->getPositionY() - 40 });
        fondo->addChild(i);

        auto btn = gd::CCMenuItemSpriteExtra::create(fondo, this, menu_selector(Creditos::onPerfil));
        btn->setUserObject(player);
        btn->setPosition({ bg->getPositionX() + offsetX, bg->getPositionY() + 45 - padY - 27 });
        menu->addChild(btn);
    }
    void onPerfil(CCObject* a) {
        auto player = reinterpret_cast<PlayerInfoObject*>(reinterpret_cast<gd::CCMenuItemSpriteExtra*>(a)->getUserObject());

        if (player->getUserID() > 0) {
            glm->m_accountIDtoUserIDDict->setObject(CCString::create(to_string(player->getUserID()).c_str()), player->getAccountID());
            glm->m_knownUsers->setObject(CCString::create(player->getUsername()), to_string(player->getUserID()));
        }

        if (player->getAccountID() > 0)
            gd::ProfilePage::create(player->getAccountID(), am->m_nPlayerAccountID == player->getAccountID() ? true : false)->show();
        else
            MensajeDesaparece::create("No tiene cuenta vinculada", 0.2f, 0.6f)->show();
    }
    void onMostrar(CCObject*) {
        Creditos::create()->show();
    }
    void agregarBoton(gd::CCMenuItemSpriteExtra* btn) {
        auto menu = reinterpret_cast<CCMenu*>(btn->getParent());
        if (auto menuLayer = reinterpret_cast<gd::MenuLayer*>(menu->getParent())) {
            menuLayer->addChild(Temporizador::create(), 99);

            auto btn = gd::CCMenuItemSpriteExtra::create(CCSprite::createWithSpriteFrameNameWithScale("communityCreditsBtn_001.png", 1.3f), menuLayer, menu_selector(Creditos::onMostrar));
            menu->addChild(btn);
        }
    }
    CREATE_FUNC(Creditos);
};

class GJComment : public CCNode {
public:
    int getAccountID() {
        return *(int*)((char*)this + 0x130);
    }
};

class CommentCell : public gd::TableViewCell {
public:
    PAD(0xC);
    GJComment* m_pComment;
};

class GJScoreCell : public gd::TableViewCell {
public:
    gd::GJUserScore* m_pScore;
};

class GJLevelScoreCell : public gd::TableViewCell {
public:
    gd::GJUserScore* getScore() {
        return *(gd::GJUserScore**)((char*)this + 0x178);
    }
};

static gd::LevelCell* levelCell;
static gd::LevelInfoLayer* levelInfoLayer;
static gd::ProfilePage* profilePage;

class NameTool {
public:
    void UpdatePauseName(CCLabelBMFont* label) {
        label->setString(gm->getPlayLayer()->m_level->m_sLevelName.c_str());
    }
    void UpdateCellName(CCLabelBMFont* label) {
        levelCell = reinterpret_cast<gd::LevelCell*>(reinterpret_cast<CCLayer*>(label->getParent())->getParent());
        label->setString(levelCell->m_pLevel->m_sLevelName.c_str());
    }
    void UpdateCellNameAccount(gd::CCMenuItemSpriteExtra* btn) {
        auto label = reinterpret_cast<CCLabelBMFont*>(btn->getChildren()->objectAtIndex(0));
        label->setTag(m_nMainTag);
        label->setPositionX(0);
        label->setAnchorPointX(0);
        label->setString(CCString::createWithFormat("De %s", levelCell->m_pLevel->m_sCreatorName.c_str())->getCString());
    }
    void UpdateShareName(CCLabelBMFont* label) {
        label->setString(reinterpret_cast<gd::EditLevelLayer*>(CCDirector::sharedDirector()->getRunningScene()->getChildren()->objectAtIndex(0))->m_pLevel->m_sLevelName.c_str());
    }
    void UpdateLevelInfoName(CCLabelBMFont* label) {
        label->setString(levelInfoLayer->m_pLevel->m_sLevelName.c_str());
    }
    void UpdateLevelInfoNameAccount(gd::CCMenuItemSpriteExtra* btn) {
        auto label = reinterpret_cast<CCLabelBMFont*>(btn->getChildren()->objectAtIndex(0));
        label->setTag(m_nMainTag);
        label->setString(CCString::createWithFormat("De %s", levelInfoLayer->m_pLevel->m_sCreatorName.c_str())->getCString());
    }
    void UpdateLevelInfoNameAccountComments(CCLabelBMFont* label) {
        label->setString(CCString::createWithFormat("Comentarios de %s", this->getUsernameByAccountID(profilePage->m_nAccountID).c_str())->getCString());
    }
    void UpdateLevelInfoLayerName(CCLabelBMFont* label) {
        levelInfoLayer = reinterpret_cast<gd::LevelInfoLayer*>(label->getParent());
        label->setString(levelInfoLayer->m_pLevel->m_sLevelName.c_str());
    }
    void UpdateLevelInfoLayerNameAccount(gd::CCMenuItemSpriteExtra* btn) {
        auto label = reinterpret_cast<CCLabelBMFont*>(btn->getChildren()->objectAtIndex(0));
        label->setTag(m_nMainTag);
        label->setString(CCString::createWithFormat("De %s", levelInfoLayer->m_pLevel->m_sCreatorName.c_str())->getCString());
    }
    void UpdateCommentCellProfile(CCLabelBMFont* label) {
        if (auto comment = reinterpret_cast<CommentCell*>(label->getParent()->getParent())->m_pComment)
            label->setString(this->getUsernameByAccountID(comment->getAccountID()).c_str());
    }
    void UpdateCommentCellProfileBtn(gd::CCMenuItemSpriteExtra* btn) {
        auto label = reinterpret_cast<CCLabelBMFont*>(btn->getChildren()->objectAtIndex(0));
        label->setTag(m_nMainTag);
        if (auto comment = reinterpret_cast<CommentCell*>(btn->getParent()->getParent()->getParent())->m_pComment)
            label->setString(this->getUsernameByAccountID(comment->getAccountID()).c_str());
    }
    void UpdateScoreCell(gd::CCMenuItemSpriteExtra* btn) {
        auto label = reinterpret_cast<CCLabelBMFont*>(btn->getChildren()->objectAtIndex(0));
        label->setTag(m_nMainTag);
        if (auto score = reinterpret_cast<GJScoreCell*>(btn->getParent()->getParent()->getParent())->m_pScore)
            label->setString(score->userName_.c_str());
    }
    void UpdateLevelLeaderboard(gd::CCMenuItemSpriteExtra* btn) {
        auto label = reinterpret_cast<CCLabelBMFont*>(btn->getChildren()->objectAtIndex(0));
        label->setTag(m_nMainTag);
        if (auto score = reinterpret_cast<GJLevelScoreCell*>(btn->getParent()->getParent()->getParent()))
            label->setString(score->getScore()->userName_.c_str());
    }
    void UpdateProfilePageName(CCLabelBMFont* label) {
        profilePage = reinterpret_cast<gd::ProfilePage*>(label->getParent()->getParent());
        label->setString(this->getUsernameByAccountID(profilePage->m_nAccountID).c_str());
    }
    string getUsernameByAccountID(int m_nAccountID) {
        if (m_nAccountID == am->m_nPlayerAccountID) return am->m_sUsername;
        int user = glm->m_accountIDtoUserIDDict->valueForKey(m_nAccountID)->intValue();
        if (user == 0) return "-";

        string nombre = reinterpret_cast<CCString*>(glm->m_knownUsers->objectForKey(CCString::createWithFormat("%i", user)->getCString()))->getCString();
        if (nombre.empty()) return "-";
        else return nombre;
    }
    void FixBackground(extension::CCScale9Sprite* bg) {
        auto flalert = reinterpret_cast<gd::FLAlertLayer*>(bg->getParent()->getParent());

        auto textArea = reinterpret_cast<CCNode*>(flalert->m_pLayer->getChildByZOrder(3));
        auto bitmapFont = reinterpret_cast<CCNode*>(textArea->getChildren()->objectAtIndex(0));

        float sizeX = bg->getContentSizeX();

        for (int i = 0; i < bitmapFont->getChildrenCount(); i++) {
            float size = reinterpret_cast<CCLabelBMFont*>(bitmapFont->getChildren()->objectAtIndex(i))->getScaledContentSizeX();
            if (size > (sizeX - 10)) sizeX = size + 25;
        }

        bg->setContentSizeX(sizeX);
    }
};

/* LEVEL NAME */ // Uso este metodo para evitar problemas con la compatibilidad de hooks ¯\_(ツ)_/¯

DWORD LevelSelectLayerBACK = base + 0x186F12;
__declspec(naked) void LevelSelectLayer() {
    __asm {
        push m_nNameTag // Tag
        push 0x00 // zOrder
        push[edi + 0x00000138] // CCLabelBMFont
        mov eax, [ecx]
        call dword ptr[eax + 0x000000D8] // [0xD8] = addChild(CCNode*, int, int) | [0xDC] = addChild(CCNode*, int) | [0xE0] = addChild(CCNode*)

        jmp[LevelSelectLayerBACK] // return to normal GD code
    }
}

DWORD PauseLayerBACK = base + 0x1E4902 + 9;
__declspec(naked) void PauseLayer() {
    __asm {
        push m_nNameTag
        push 0x00
        push edi
        mov eax, [ebx]
        call dword ptr[eax + 0x000000D8]
        push ecx
        push edi
        call NameTool::UpdatePauseName
        pop ecx
        jmp[PauseLayerBACK]
    }
}

DWORD LevelCellLocalBACK = base + 0x5BECB;
__declspec(naked) void LevelCellLocal() {
    __asm {
        push m_nNameTag
        push 00
        push edi
        mov[ebp - 0x14], 0x0000000F
        mov[ebp - 0x18], 0x00000000
        mov eax, [ecx]
        mov byte ptr[ebp - 0x28], 0x00
        call dword ptr[eax + 0x000000D8]
        push ecx
        push edi
        call NameTool::UpdateCellName
        pop ecx
        jmp[LevelCellLocalBACK]
    }
}

DWORD LevelCellOnlineBACK = base + 0x5AA0D;
__declspec(naked) void LevelCellOnline() {
    __asm {
        push m_nNameTag
        push 00
        push edi
        mov[ebp - 0x2C], 0x0000000F
        mov[ebp - 0x30], 0x00000000
        mov eax, [ecx]
        mov byte ptr[ebp - 0x40], 0x00
        call dword ptr[eax + 0x000000D8]
        push ecx
        push edi
        call NameTool::UpdateCellName
        pop ecx
        jmp[LevelCellOnlineBACK]
    }
}

DWORD CCTextInputNodeBACK = base + 0x20F93 + 7;
__declspec(naked) void CCTextInputNode() {
    __asm {
        push m_nNameTag
        push 0x00
        push eax
        call dword ptr[edx + 0x000000D8]
        jmp[CCTextInputNodeBACK]
    }
}

DWORD ShareLevelLayerBACK = base + 0x24D520;
__declspec(naked) void ShareLevelLayer() {
    __asm {
        push m_nNameTag
        push 0x00
        push edi
        mov[esp + 0x000000D0], 0x0000000F
        mov[esp + 0x000000CC], 0x00000000
        mov eax, [ecx]
        mov byte ptr[esp + 0x000000BC], 0x00
        call dword ptr[eax + 0x000000D8]
        push ecx
        push edi
        call NameTool::UpdateShareName
        pop ecx
        jmp[ShareLevelLayerBACK]
    }
}

DWORD LevelInfoBACK = base + 0x14FC8E;
__declspec(naked) void LevelInfo() {
    __asm {
        push m_nNameTag
        push 0x14
        push[ebp - 0x00000084]
        mov ecx, [ebx + 0x000001AC]
        mov eax, [ecx]
        call dword ptr[eax + 0x000000D8]
        push ecx
        push[ebp - 0x00000084]
        call NameTool::UpdateLevelInfoName
        pop ecx
        jmp[LevelInfoBACK]
    }
}

DWORD LevelInfoLayerBACK = base + 0x176213;
__declspec(naked) void LevelInfoLayer() {
    __asm {
        push m_nNameTag
        push 0x14
        push esi
        mov[ebp - 0x30], 0x0000000F
        mov[ebp - 0x34], 0x00000000
        mov byte ptr[ebp - 0x44], 0x00
        call dword ptr[eax + 0x000000D8]
        push ecx
        push esi
        call NameTool::UpdateLevelInfoLayerName
        pop ecx
        jmp[LevelInfoLayerBACK]
    }
}

/* USERNAMES */

DWORD MenuLayerBACK = base + 0x191537 + 9;
__declspec(naked) void MenuLayer() {
    __asm {
        push m_nMainTag
        push 0x02
        push eax
        call dword ptr[edx + 0x000000D8]
        jmp[MenuLayerBACK]
    }
}

DWORD ProfilePageBACK = base + 0x20F2AA + 9;
__declspec(naked) void ProfilePage() {
    __asm {
        push m_nMainTag
        push 0x14
        push eax
        call dword ptr[edx + 0x000000D8]
        push ecx
        call NameTool::UpdateProfilePageName
        jmp[ProfilePageBACK]
    }
}

DWORD LevelCellOnlineCuentaBACK = base + 0x5B030 + 9;
__declspec(naked) void LevelCellOnlineCuenta() {
    __asm {
        push m_nMainTag
        push 0x00
        push esi
        mov edx, [eax]
        call dword ptr[edx + 0x000000D8]
        push ecx
        push esi
        call NameTool::UpdateCellNameAccount
        pop ecx
        jmp[LevelCellOnlineCuentaBACK]
    }
}

DWORD LevelInfoLayerCuentaBACK = base + 0x1764EA + 9;
__declspec(naked) void LevelInfoLayerCuenta() {
    __asm {
        push m_nMainTag
        push 0x00
        push esi
        mov edx, [eax]
        call dword ptr[edx + 0x000000d8]
        push ecx
        push esi
        call NameTool::UpdateLevelInfoLayerNameAccount
        pop ecx
        jmp[LevelInfoLayerCuentaBACK]
    }
}

DWORD LevelInfoCuentaBACK = base + 0x14FCAE + 7;
__declspec(naked) void LevelInfoCuenta() {
    __asm {
        push m_nMainTag
        push 0x00
        push esi
        call dword ptr[edx + 0x000000D8]
        push ecx
        push esi
        call NameTool::UpdateLevelInfoNameAccount
        pop ecx
        jmp[LevelInfoCuentaBACK]
    }
}

DWORD LevelInfoCommentsBACK = base + 0x14F8BB;
__declspec(naked) void LevelInfoComments() {
    __asm {
        push m_nMainTag
        push 0x00
        push ebx
        mov ecx, [eax + 0x000001AC]
        mov eax, [ecx]
        call dword ptr[eax + 0x000000D8]
        push ecx
        push ebx
        call NameTool::UpdateLevelInfoNameAccountComments
        pop ecx
        jmp[LevelInfoCommentsBACK]
    }
}

DWORD CommentCellProfileBACK = base + 0x5FD09 + 9;
__declspec(naked) void CommentCellProfile() {
    __asm {
        push m_nMainTag
        push 0x00
        push esi
        mov eax, [ecx]
        call dword ptr[eax + 0x000000D8]
        push ecx
        push esi
        call NameTool::UpdateCommentCellProfile
        pop ecx
        jmp[CommentCellProfileBACK]
    }
}

DWORD CommentCellProfileBtnBACK = base + 0x5FC46 + 9;
__declspec(naked) void CommentCellProfileBtn() {
    __asm {
        push m_nMainTag
        push 0x00
        push edi
        mov edx, [eax]
        call dword ptr[edx + 0x000000D8]
        push ecx
        push edi
        call NameTool::UpdateCommentCellProfileBtn
        pop ecx
        jmp[CommentCellProfileBtnBACK]
    }
}

DWORD ScoreCellBACK = base + 0x617B0 + 7;
__declspec(naked) void ScoreCell() {
    __asm {
        push m_nMainTag
        push 0x00
        push eax
        call dword ptr[edx + 0x000000D8]
        push ecx
        call NameTool::UpdateScoreCell
        jmp[ScoreCellBACK]
    }
}

DWORD LevelLeaderboardBACK = base + 0x6293E + 9;
__declspec(naked) void LevelLeaderboard() {
    __asm {
        push m_nMainTag
        push 0x00
        push edi
        mov edx, [eax]
        call dword ptr[edx + 0x000000D8]
        push ecx
        push edi
        call NameTool::UpdateLevelLeaderboard
        pop ecx
        jmp[LevelLeaderboardBACK]
    }
}

DWORD CreditsBtnBACK = base + 0x190BCA + 6;
__declspec(naked) void CreditsBtn() {
    __asm {
        call dword ptr[edx + 0x000000E0]
        push ecx
        call Creditos::agregarBoton
        jmp[CreditsBtnBACK]
    }
}

DWORD FLAlertLayerBGBACK = base + 0x22C1D + 6;
__declspec(naked) void FLAlertLayerBG() {
    __asm {
        call dword ptr[eax + 0x000000DC]
        push ecx
        push edi
        call NameTool::FixBackground
        pop ecx
        jmp[FLAlertLayerBGBACK]
    }
}

const char* descripcion = "Esta funcion sirve para evitar que el juego se traduzca, se quedara en <cb>ingles</c> hasta que desactives esta opcion.";
const char* variable = "TraducirGD";
const char* toggleTitulo = "No traducir el GD";

const char* descripcion2 = "Esta funcion sirve para que se traduzcan los nombres de los niveles (a peticion de SpaceDream).";
const char* variable2 = "TraducirNiveles";
const char* toggleTitulo2 = "Traducir los nombres\nde los niveles";

DWORD addToggleCall = base + 0x1DF6B0;

DWORD AddToggleBACK = base + 0x1DEEB0 + 5;
__declspec(naked) void AddToggle() {
    __asm {
        call addToggleCall
        push descripcion
        push variable
        push toggleTitulo
        mov ecx, esi
        call addToggleCall
        push descripcion2
        push variable2
        push toggleTitulo2
        mov ecx, esi
        call addToggleCall

        jmp[AddToggleBACK]
    }
}

__declspec(dllexport) void codigoDeSai() {
    /*
       ESTO ES PARA EVITAR QUE EL JUEGO TRADUZCA LOS NOMBRES DE LOS NIVELES Y DE LAS CUENTAS EN ALGUNAS ESCENAS
       basicamente lo que hago es que usando Assembly cambio el CCNode::addChild(CCNode*) del CCLabelBMFont / CCMenuItemSpriteExtra a CCNode::addchild(CCNode*, int, int)
       y le agrego un tag, en este caso el m_nMainTag que es igual a 7777. Debo refrescar la label porque si no se queda la anterior traduccion
       por eso hice una clase especial para eso "NameTool".

       Todo esto de aca no estaba en la v1.0, asi que ahora el tema del porteo a la 2.2 o para android sera mas complicado por tener que buscar todas las direcciones xd
    */

    Salto((BYTE*)base + 0x1DEEB0, (DWORD)AddToggle, 5);
    Salto((BYTE*)base + 0x22C1D, (DWORD)FLAlertLayerBG, 6);
    Salto((BYTE*)base + 0x190BCA, (DWORD)CreditsBtn, 6);
    Salto((BYTE*)base + 0x6293E, (DWORD)LevelLeaderboard, 9);
    Salto((BYTE*)base + 0x617B0, (DWORD)ScoreCell, 7);
    Salto((BYTE*)base + 0x5FC46, (DWORD)CommentCellProfileBtn, 9);
    Salto((BYTE*)base + 0x5FD09, (DWORD)CommentCellProfile, 9);
    Salto((BYTE*)base + 0x14F8AC, (DWORD)LevelInfoComments, 7);
    Salto((BYTE*)base + 0x14FCAE, (DWORD)LevelInfoCuenta, 7);
    Salto((BYTE*)base + 0x1764EA, (DWORD)LevelInfoLayerCuenta, 9);
    Salto((BYTE*)base + 0x5B030, (DWORD)LevelCellOnlineCuenta, 9);
    Salto((BYTE*)base + 0x20F2AA, (DWORD)ProfilePage, 9);
    Salto((BYTE*)base + 0x191537, (DWORD)MenuLayer, 9);
    Salto((BYTE*)base + 0x1761F8, (DWORD)LevelInfoLayer, 10);
    Salto((BYTE*)base + 0x14FC78, (DWORD)LevelInfo, 8);
    Salto((BYTE*)base + 0x24D4F9, (DWORD)ShareLevelLayer, 12);
    Salto((BYTE*)base + 0x20F93, (DWORD)CCTextInputNode, 7);
    Salto((BYTE*)base + 0x5A9F2, (DWORD)LevelCellOnline, 8);
    Salto((BYTE*)base + 0x5BEB0, (DWORD)LevelCellLocal, 5);
    Salto((BYTE*)base + 0x1E4902, (DWORD)PauseLayer, 9);
    Salto((BYTE*)base + 0x186F04, (DWORD)LevelSelectLayer, 6);

    CREAR_HOOK_LIB(SetString, _SetString, libcocosbase, 0x9FB60, hookSetString, 6);
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD  ul_reason_for_call, LPVOID lpReserved) {
    if (ul_reason_for_call == DLL_PROCESS_ATTACH)
        CreateThread(nullptr, 0, (LPTHREAD_START_ROUTINE)codigoDeSai, hModule, 0, nullptr);
    return TRUE;
}