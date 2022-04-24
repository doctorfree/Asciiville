#pragma once

#include <string>

class MenuSelectOptionElement {
  private:
    std::string identifier;
    int col;
    int row;
    bool shown;
    bool selectable;
    unsigned int id;
    std::string extradata;
    void * origin;
  protected:
    std::string label;
    bool active;
  public:
    void init(const std::string& identifier, int row, int col, const std::string& label);
    virtual ~MenuSelectOptionElement();
    void setPosition(int row, int col);
    virtual std::string getLabelText() const;
    std::string getIdentifier() const;
    unsigned int getId() const;
    std::string getExtraData() const;
    virtual std::string getContentText() const = 0;
    virtual unsigned int getTotalWidth() const;
    virtual bool activate();
    virtual void deactivate();
    virtual bool isActive() const;
    virtual int cursorPosition() const;
    virtual bool inputChar(int ch);
    void setOrigin(void* origin);
    void* getOrigin() const;
    virtual std::string getLegendText() const;
    unsigned int getCol() const;
    unsigned int getRow() const;
    void hide();
    void show();
    bool visible() const;
    bool isSelectable() const;
    void setSelectable(bool);
    void setId(unsigned int);
    void setExtraData(const std::string& data);
    void setLabel(const std::string& label);
};
