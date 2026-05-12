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
int main() {
	EM_ASM({
		Module.domCache = {};
	});
	
	Router::add("/",page_1);
    int kl = 44;
	cout <<  << endl;
	page_1->builder = [&, kl](VPage& page, std::string msg) {
		page.setTitle("Helios ~ Your Full Stack Framework");
		page.addScript("anim.js");
		page.addStylesheet("./global.css");
		page.addStylesheet("https://cdn.lineicons.com/5.1/line/lineicons.css");
		page.setFavicon("logo.png");

		auto h = Platform().height();
			if(h > 992.0){
    	cout << string("This is a desktop device") << endl;
    
    }
    else {	cout << string("This is a mobile device") << endl;
    }
		
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
