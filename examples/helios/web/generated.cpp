#include <iostream>
#include "vdom.hpp"
#include <format>
#include <cmath>
#include <vector>
#include <any>
using namespace std;

void updateUI() {
        // Re-render the current page
        if (GlobalState::getCurrentPage()) {
            GlobalState::getCurrentPage()->render(true);
        }
    }
auto page_1 = make_shared<VPage>();

auto page_2 = make_shared<VPage>();
int main() {
	EM_ASM({
		Module.domCache = {};
	});
	
	Router::add("/",page_1);
	Router::add("/terminal",page_2);
	page_1->builder = [&](VPage& page, std::string msg) {
		page.setTitle("MAINNNN");
		page.addScript("anim.js");
		page.addStylesheet("./global.css");
		page.addStylesheet("https://cdn.lineicons.com/5.1/line/lineicons.css");
		page.setFavicon("logo.png");

		auto hei = Platform().height();
		
		VNode text_2("p","Welcome to Helios!", "__ink_4");

	page.addChild(text_2);
		};
	page_2->builder = [&](VPage& page, std::string msg) {
		page.setTitle("Helios ~ Your Full Stack Framework");
		page.addStylesheet("./global.css");
		page.addStylesheet("https://cdn.lineicons.com/5.1/line/lineicons.css");
		page.setFavicon("logo.png");

		auto hei = Platform().height();
		page.addevent("resize", [&]() {
			cout << hei << endl;
updateUI();
		});

		
		VNode text_3("p","Welcome to terminal page!", "__ink_5");

	page.addChild(text_3);
		};
	EM_ASM({
		Module._handleRoute(allocateUTF8(window.location.pathname));
		window.addEventListener("popstate", () => {
		Module._handleRoute(allocateUTF8(window.location.pathname));
		});
	});return 0;
}
