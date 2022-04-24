#pragma once

#include <memory>
#include <string>
#include <vector>

#include "focusablearea.h"

class MenuSelectOptionElement;
class MenuSelectOptionTextArrow;
class MenuSelectAdjustableLine;
class MenuSelectOptionTextButton;
class MenuSelectOptionTextField;
class MenuSelectOptionNumArrow;
class MenuSelectOptionCheckBox;
class VirtualView;

class MenuSelectOption : public FocusableArea {
  private:
    bool navigate(int);
    unsigned int pointer;
    unsigned int lastpointer;
    std::vector<std::shared_ptr<MenuSelectOptionElement> > options;
    std::vector<std::shared_ptr<MenuSelectAdjustableLine> > adjustablelines;
    VirtualView& vv;
    bool affectvvvertical;
  public:
    MenuSelectOption(VirtualView& vv, bool affectvvvertical = true);
    ~MenuSelectOption();
    bool goDown();
    bool goUp();
    bool goRight();
    bool goLeft();
    bool goNext();
    bool goPrevious();
    std::shared_ptr<MenuSelectOptionTextField> addStringField(int, int, std::string, std::string, std::string, bool);
    std::shared_ptr<MenuSelectOptionTextField> addStringField(int, int, std::string, std::string, std::string, bool, int);
    std::shared_ptr<MenuSelectOptionTextField> addStringField(int, int, std::string, std::string, std::string, bool, int, int);
    std::shared_ptr<MenuSelectOptionTextArrow> addTextArrow(int, int, std::string, std::string);
    std::shared_ptr<MenuSelectOptionNumArrow> addIntArrow(int, int, std::string, std::string, int, int, int);
    std::shared_ptr<MenuSelectOptionCheckBox> addCheckBox(int, int, std::string, std::string, bool);
    std::shared_ptr<MenuSelectOptionTextButton> addTextButton(int, int, std::string, std::string);
    std::shared_ptr<MenuSelectOptionTextButton> addTextButtonNoContent(int, int, std::string, std::string);
    std::shared_ptr<MenuSelectAdjustableLine> addAdjustableLine();
    std::shared_ptr<MenuSelectAdjustableLine> addAdjustableLineBefore(std::shared_ptr<MenuSelectAdjustableLine>);
    std::shared_ptr<MenuSelectOptionElement> getElement(unsigned int) const;
    std::shared_ptr<MenuSelectOptionElement> getElement(std::string) const;
    std::vector<std::shared_ptr<MenuSelectAdjustableLine> >::iterator linesBegin();
    std::vector<std::shared_ptr<MenuSelectAdjustableLine> >::iterator linesEnd();
    std::shared_ptr<MenuSelectAdjustableLine> getAdjustableLine(std::shared_ptr<MenuSelectOptionElement>) const;
    std::shared_ptr<MenuSelectAdjustableLine> getAdjustableLine(unsigned int lineindex) const;
    std::shared_ptr<MenuSelectAdjustableLine> getAdjustableLineOnRow(unsigned int row) const;
    bool swapLineWithNext(std::shared_ptr<MenuSelectAdjustableLine>);
    bool swapLineWithPrevious(std::shared_ptr<MenuSelectAdjustableLine>);
    int getLineIndex(std::shared_ptr<MenuSelectAdjustableLine>);
    void removeAdjustableLine(std::shared_ptr<MenuSelectAdjustableLine>);
    void removeElement(std::shared_ptr<MenuSelectOptionElement>);
    void setPointer(std::shared_ptr<MenuSelectOptionElement>);
    unsigned int getLastSelectionPointer() const;
    unsigned int getSelectionPointer() const;
    bool activateSelected();
    unsigned int size() const;
    unsigned int linesSize() const;
    void adjustLines(unsigned int);
    void enterFocusFrom(int);
    void clear();
    void checkPointer();
    void reset();
};
