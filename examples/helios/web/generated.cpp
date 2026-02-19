#include <iostream>
#include "vdom.hpp"
#include <format>
#include <cmath>
#include <list>
#include <any>
using namespace std;

void updateUI() {
        // Re-render the current page
        if (GlobalState::getCurrentPage()) {
            GlobalState::getCurrentPage()->render(true);
        }
    }
auto page_1 = make_shared<VPage>();
int main() {
	Router::add("/",page_1);
std::list<std::any> listofpages;
	listofpages.push_back(string("index.ink"));
	listofpages.push_back(string("about.ink"));
	listofpages.push_back(string("contact.ink"));
	page_1->builder = [&, listofpages](VPage& page, std::string msg) {
		page.setTitle("Helios ~ Your Full Stack Framework");
		page.addScript("anim.js");
		page.addStylesheet("./global.css");
		page.addStylesheet("https://cdn.lineicons.com/5.1/line/lineicons.css");
		page.setFavicon("logo.png");

		for (    int p = 0;(p < listofpages.size());p++){
		cout <<  << endl;}
		
		VNode text_2("p","Welcome to Helios!", "__ink_0");

	page.addChild(text_2);
		};
	EM_ASM({
		Module._handleRoute(allocateUTF8(window.location.pathname));
		window.addEventListener("popstate", () => {
		Module._handleRoute(allocateUTF8(window.location.pathname));
		});
	});return 0;
}
