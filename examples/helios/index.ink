
page("Helios ~ Your Full Stack FrmeWork", style={
    "background-color": "black",
    "color": "white",
    "padding": "0px",
    "margin": "0px"
}) {
    
    h = Platform().height
    w = Platform().width
    @state num : 0
    canvas("bg", height=h, width=w)
    img("box-bg.jpg", style={ 

        "position": "absolute",
        "top": "50px",
        "left": "0px",
        "width": "100%",
        "height": "100%",
        "object-fit": "cover",
        "z-index": "1",
        "opacity": "0.4"
    })
 
    view("mydiv", style={
        "height": "50px",
        "background-color": "gray"
    }, onclick=(num) { 
        num = num + 1
        print(num) 
    }) {
        text(to_str(num), style={
            "padding": "0px",
            "margin": "0px",
            "font-weight": "bold",
            "font-size": "20px",
            "line-height": "20px",
            "text-align": "center",
            "user-select": "none",
            "cursor": "pointer"
        })
        text("reload")
    } 

}
        