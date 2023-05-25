#include <QCoreApplication>

#include "CommandLineParser.h"

#include <iostream>
#include <string>
#include<conio.h>

#include "githashobject.h"

using namespace std;


int main(int argc, char *argv[])
{
    GitHashObject obj;

    int nthTabPressed = 0;
    const auto tabcode = 0x00001001;
    CommandLineParser p;
    QCoreApplication a(argc, argv);
    a.installEventFilter(&p);
    std::string s;
    p.printWorkingDirectory();
    while(true){
      char c = 'a';
      string ipt = "";
      string tabSubText = "";
      string tabAutoComplete = "";
      char ipt_ch;
      bool tabPressed = false;
      int tabAddedSybolsCount = 0;
      while (true) {
          ipt_ch = getch();

          // Check whether user enters
          // a special non-printable
          // character

          if(ipt_ch != 9){
              tabPressed = false;
              nthTabPressed = 0;
              p.setNthTabSelectionIndex(nthTabPressed);
          }
          if (ipt_ch == 9) {
              // tab
              if(tabPressed){
                  nthTabPressed++;
              }
              tabPressed = true;
              p.setNthTabSelectionIndex(nthTabPressed);
              tabAutoComplete  = p.process(ipt, true);
              {
                  auto space = ipt.find_last_of(' ');
                  if(space > 0){
                      auto part = ipt.substr(space + 1, ipt.size() - space);
//                      std::cout<< "<part=" << part << ";      ";
                      tabSubText = part;
                      for(int _ = 0; _ < tabAddedSybolsCount; _++){
                          std::cout << '\b';
//                            ipt.pop_back();
                      }
                      for(int _ = 0; _ < tabAddedSybolsCount; _++){
                          std::cout << ' ';
//                            ipt.pop_back();
                      }
                      for(int _ = 0; _ < tabAddedSybolsCount; _++){
                          std::cout << '\b';
//                            ipt.pop_back();
                      }
                  }
              }
              if(!tabAutoComplete .empty()){

                  tabAddedSybolsCount = tabAutoComplete .length() - tabSubText.length();
                  std::cout<< tabAutoComplete.substr(tabSubText.length());
//                  ipt.append(autoComplete);
              }
              continue;
              break;
          }
          else if(ipt_ch == 8){
            // backspace

              if(tabAddedSybolsCount > 0){
                  tabAddedSybolsCount--;
                  std::cout << '\b' << ' ' << '\b';
                  tabAutoComplete .pop_back();
              }
              else if(!ipt.empty()){
                  std::cout << '\b' << ' ' << '\b';
                  ipt.pop_back();
              }
               continue;

          }
          else if(ipt_ch == 13){
              std::cout << std::endl;
              if(!ipt.empty()){
                  if(tabAddedSybolsCount > 0){
                      ipt.replace(ipt.find(tabSubText), tabSubText.length(), tabAutoComplete);
                      tabAddedSybolsCount = 0;
                      tabSubText.clear();
                      tabAutoComplete.clear();
                  }
                  p.process(ipt);
              }
              p.printWorkingDirectory();
              ipt.clear();
              continue;
          }
          else{
              cout << ipt_ch;
//              cout << ipt_ch << "<" << (int)ipt_ch<< ">";

          }
          ipt.push_back(ipt_ch);
      }

        std::getline(std::cin, s);
        auto autoComplete = p.process(s);
        p.printWorkingDirectory();
    }



    return a.exec();
}
