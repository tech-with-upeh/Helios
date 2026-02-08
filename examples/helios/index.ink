@stylesheet homepage {
    mydiv =  {
        height: "50px;",
        width: "100%;",
        position: "fixed;",
        top: "0px;",
        display: "flex;",
        "align-items": "center;",
        "justify-content": "space-between;",
        "border-bottom-right-radius": "20px;",
        "border-bottom-left-radius": "20px;",
        "z-index": "999"
    }

    brandlogo = {
        height: "30px;",
        width: "30px;",
        "object-fit": "contain;"

    }
    
    mydivtext = {
        "font-weight": "bold;",
        "font-size": "20px;",
        "text-align": "center;",
        "user-select": "none;",
        "cursor": "pointer;"
    }
    bgimg = {
        "position": "absolute",
        "top": "0",
        "left": "0px",
        "width": "100%",
        "height": "100%",
        "object-fit": "cover",
        "z-index": "1",
        "opacity": "0.2"
    }
}

page("Helios ~ Your Full Stack FrmeWork", style={
    "background-color": "black",
    "color": "white",
    "padding": "0px",
    "margin": "0px"
}) {
    addStyle(homepage)
    h = Platform().height
    w = Platform().width
    @state num : 0
    @state navh : "0px"
    @state navpad : "0px"
    canvas("bg", height=h, width=w)
    img("box-bg.jpg", cls="bgimg")
 
    view("mydiv", cls="mydiv", onclick=(num) { 
        num = num + 1
        print(num) 
    }) {
        view("nav-start", cls="navstart") {
            img("logo-wh.png", cls="brandlogo")
            text("Helios", cls="brandname")
        }
        view("nav-end", cls="navend") {
            text(to_str(num), onclick=(navh, navpad) {
                if navh == "0px" {
                    navh = "150px"
                }else {
                    navh = "0px"
                }
            }, cls="mydivtext")
            img("nav.png", cls="navicon")
            view("nav-menu",style={
                "padding": str(navpad) + "px;",
                "height": str(navh) + "px;"
        }, cls="navmenu") {
                text("Home")
                text("Docs")
                text("GitHub")
            }
        }
    } 

}
        