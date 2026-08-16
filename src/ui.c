
#include "ui.h" //pulls in our own header
#include <stdio.h>

void InitUI(void) {
    // Reserved for future UI resources (fonts, sounds)
}//empty for now. kichu korena.

void DrawUIPanel(Rectangle bounds, Color background , Color border, float borderWidth){
    DrawRectangleRec(bounds, background);
    DrawRectangleLinesEx(bounds, borderWidth, border);
}

bool DrawUIButton(Rectangle bounds, const char *text, Color baseColor, Color hoverColor){ //bool: এই ফাংশনটি true (সত্য) অথবা false (মিথ্যা) ফেরত দেবে। বাটনে ক্লিক হলে true দেবে, না হলে false দেবে।

    //mouse koi ase
    Vector2 mousePos = GetMousePosition(); //vector2 is raylib's struct for an {x, y} point. GetMousePosition() asks raylib "where is the cursor right now, in window pixels" and hands back that point. This is called fresh every single frame — raylib doesn't track "the mouse" persistently for you, your game loop asks anew 60 times a second.
    //mous er pointer vitore ase kina box er taile hovered
    bool isHovered = CheckCollisionPointRec(mousePos, bounds); //a raylib collision-detection helper: "is this point inside this rectangle
    //jodi thake hoverColor naile arekta
    Color activeColor = isHovered ? hoverColor : baseColor;

    DrawRectangleRec(bounds, activeColor);
    DrawRectangleLinesEx(bounds, 2.0f, DARKGRAY);

    int fontSize = 18;
    int textWidth = MeasureText(text, fontSize);
    float textX = bounds.x + (bounds.width - textWidth) / 2.0f; //
    float textY = bounds.y + (bounds.height - fontSize) / 2.0f;

    DrawText(text, (int)textX, (int)textY, fontSize, WHITE);

    return (isHovered && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)); //মাউসটি যদি বাটনের ওপর থাকে (isHovered) এবং মাউসের বামের বাটনে ক্লিক করা হয় (IsMouseButtonPressed), কেবল তখনই এটি true রিটার্ন করবে।


}