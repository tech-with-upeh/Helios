#include <iostream>
#include "vdom.hpp"
#include <format>
#include <cmath>
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
	page_1->builder = [&](VPage& page) {
		page.setTitle("Helios ~ Your Full Stack FrmeWork");
		page.bodyAttrs["style"] = "background-color:black;color:white;padding:0px;margin:0px;";
		page.addScript("anim.js");
		page.addStylesheet("./global.css");
		page.setFavicon("logo.png");

		auto h = Platform().height();
		auto w = Platform().width();
		
	auto num = make_shared<appstate::State<int>>("num",0);
		
	VNode canvas_1("canvas", "", "__ink_10");
		canvas_1.type = VNodeType::CANVAS;
		canvas_1.setAttr("id", "bg");
		canvas_1.height = h;
		canvas_1.width = w;

	page.addChild(canvas_1);
		
	VNode img_2("img", "", "__ink_11");
	img_2.setAttr("src", "box-bg.jpg");
	img_2.setAttr("style", "position:absolute;top:50px;left:0px;width:100%;height:100%;object-fit:cover;z-index:1;opacity:0.4;");

	page.addChild(img_2);
		
	VNode view_3("div", "", "__ink_12");
		view_3.setAttr("id", "mydiv");
	view_3.onClick([num]() {
			num->set((num->get() + 1));
		cout << num->get() << endl;
		updateUI();	});
	view_3.setAttr("style", "height:50px;background-color:gray;");

		VNode text_3("p",to_string(num->get()), "__ink_13");
	text_3.setAttr("style", "padding:0px;margin:0px;font-weight:bold;font-size:20px;line-height:20px;text-align:center;user-select:none;cursor:pointer;");

	view_3.addChild(text_3);
		VNode text_4("p","reload", "__ink_14");

	view_3.addChild(text_4);
	page.addChild(view_3);
		};
	EM_ASM({
		Module._handleRoute(allocateUTF8(window.location.pathname));
		window.addEventListener("popstate", () => {
		Module._handleRoute(allocateUTF8(window.location.pathname));
		});
	});return 0;
}
