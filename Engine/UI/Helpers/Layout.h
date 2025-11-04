#pragma once

#include "../Core/UITypes.h"
#include "../Components/Canvas.h"
#include "../Components/RectTransform.h"
#include "../../Math/Math.h"

namespace Uma_UI
{
    /**
     * \brief Compute canvas scale factor based on screen resolution
     * \param canvas Canvas component
     * \param screenWidth Current screen width in pixels
     * \param screenHeight Current screen height in pixels
     * \return Scale factor to apply to UI elements
     */
    inline float ComputeCanvasScale(const Canvas& canvas, float screenWidth, float screenHeight)
    {
        if (canvas.scaleMode == CanvasScaleMode::ConstantPixelSize)
        {
            return 1.0f;
        }

        // ScaleWithScreenSize
        float widthScale = screenWidth / canvas.referenceResolution.x;
        float heightScale = screenHeight / canvas.referenceResolution.y;

        // Lerp between width and height based on matchWidthOrHeight
        float t = canvas.matchWidthOrHeight;
        return widthScale * (1.0f - t) + heightScale * t;
    }

    /**
     * \brief Convert pixel coordinates to NDC [-1, 1]
     * \param pixelPos Position in pixels (origin bottom-left)
     * \param screenWidth Screen width in pixels
     * \param screenHeight Screen height in pixels
     * \return Position in NDC space
     */
    inline Vec2 PixelToNDC(const Vec2& pixelPos, float screenWidth, float screenHeight)
    {
        float ndcX = (pixelPos.x / screenWidth) * 2.0f - 1.0f;
        float ndcY = (pixelPos.y / screenHeight) * 2.0f - 1.0f;
        return Vec2(ndcX, ndcY);
    }

    /**
     * \brief Convert NDC coordinates to pixels
     * \param ndcPos Position in NDC [-1, 1]
     * \param screenWidth Screen width in pixels
     * \param screenHeight Screen height in pixels
     * \return Position in pixel space (origin bottom-left)
     */
    inline Vec2 NDCToPixel(const Vec2& ndcPos, float screenWidth, float screenHeight)
    {
        float pixelX = (ndcPos.x + 1.0f) * 0.5f * screenWidth;
        float pixelY = (ndcPos.y + 1.0f) * 0.5f * screenHeight;
        return Vec2(pixelX, pixelY);
    }

    /**
     * \brief Convert pixel size to NDC size
     * \param pixelSize Size in pixels
     * \param screenWidth Screen width in pixels
     * \param screenHeight Screen height in pixels
     * \return Size in NDC space
     */
    inline Vec2 PixelSizeToNDC(const Vec2& pixelSize, float screenWidth, float screenHeight)
    {
        float ndcW = (pixelSize.x / screenWidth) * 2.0f;
        float ndcH = (pixelSize.y / screenHeight) * 2.0f;
        return Vec2(ndcW, ndcH);
    }

    /**
     * \brief Get screen-space rect (full screen in NDC)
     * \return NDC rect covering entire screen centered at origin
     */
    inline Rect GetScreenRect()
    {
        // FIXED: Center at (0,0), size (2.0, 2.0) covers [-1,1] in both axes
        return Rect(0.0f, 0.0f, 2.0f, 2.0f);
    }

    /**
     * \brief Compute final NDC rect from RectTransform + parent rect + canvas scale
     * \param rectTransform RectTransform to compute
     * \param parentRect Parent's computed NDC rect (or screen rect if root)
     * \param canvasScale Scale factor from canvas
     * \param screenWidth Screen width in pixels
     * \param screenHeight Screen height in pixels
     * \return Computed NDC rectangle
     */
    inline Rect ComputeRectInNDC(
        const RectTransform& rectTransform,
        const Rect& parentRect,
        float canvasScale,
        float screenWidth,
        float screenHeight)
    {
        // 1. Get parent bounds in NDC
        float parentLeft = parentRect.Left();
        float parentRight = parentRect.Right();
        float parentBottom = parentRect.Bottom();
        float parentTop = parentRect.Top();
        float parentWidth = parentRect.width;
        float parentHeight = parentRect.height;

        // 2. Compute anchor positions in parent's NDC space
        Vec2 anchorMinNDC(
            parentLeft + rectTransform.anchorMin.x * parentWidth,
            parentBottom + rectTransform.anchorMin.y * parentHeight
        );

        Vec2 anchorMaxNDC(
            parentLeft + rectTransform.anchorMax.x * parentWidth,
            parentBottom + rectTransform.anchorMax.y * parentHeight
        );

        // 3. Convert sizeDelta from pixels to NDC
        Vec2 sizePixels = rectTransform.sizeDelta * canvasScale;
        Vec2 sizeNDC = PixelSizeToNDC(sizePixels, screenWidth, screenHeight);

        // 4. Convert anchoredPosition from pixels to NDC
        Vec2 anchoredPosPixels = rectTransform.anchoredPosition * canvasScale;
        // FIXED: Convert offset correctly - it's a relative offset, not absolute position
        Vec2 anchoredPosNDC = PixelSizeToNDC(anchoredPosPixels, screenWidth, screenHeight);

        // 5. Compute final rect based on anchor mode
        float finalWidthNDC;
        float finalHeightNDC;
        float finalCenterX;
        float finalCenterY;

        // Horizontal computation
        if (rectTransform.IsStretchingHorizontal())
        {
            // Stretching: width = distance between anchors + sizeDelta offsets
            float anchorSpanNDC = anchorMaxNDC.x - anchorMinNDC.x;
            finalWidthNDC = anchorSpanNDC + sizeNDC.x;
            // Center between anchors plus offset
            finalCenterX = (anchorMinNDC.x + anchorMaxNDC.x) * 0.5f + anchoredPosNDC.x;
        }
        else
        {
            // Not stretching: fixed width from sizeDelta
            finalWidthNDC = sizeNDC.x;
            // Position at anchor point plus offset, adjusted by pivot
            float anchorX = anchorMinNDC.x;  // anchorMin == anchorMax when not stretching
            finalCenterX = anchorX + anchoredPosNDC.x + finalWidthNDC * (0.5f - rectTransform.pivot.x);
        }

        // Vertical computation
        if (rectTransform.IsStretchingVertical())
        {
            // Stretching: height = distance between anchors + sizeDelta offsets
            float anchorSpanNDC = anchorMaxNDC.y - anchorMinNDC.y;
            finalHeightNDC = anchorSpanNDC + sizeNDC.y;
            // Center between anchors plus offset
            finalCenterY = (anchorMinNDC.y + anchorMaxNDC.y) * 0.5f + anchoredPosNDC.y;
        }
        else
        {
            // Not stretching: fixed height from sizeDelta
            finalHeightNDC = sizeNDC.y;
            // Position at anchor point plus offset, adjusted by pivot
            float anchorY = anchorMinNDC.y;  // anchorMin == anchorMax when not stretching
            finalCenterY = anchorY + anchoredPosNDC.y + finalHeightNDC * (0.5f - rectTransform.pivot.y);
        }

        // 6. Return final rect with center and size
        return Rect(finalCenterX, finalCenterY, finalWidthNDC, finalHeightNDC);
    }

    /**
     * \brief Convert NDC position to screen pixels
     * \param ndcX X position in NDC
     * \param ndcY Y position in NDC
     * \param screenWidth Screen width in pixels
     * \param screenHeight Screen height in pixels
     * \return Position in pixel space
     */
    inline Vec2 NDCToScreen(float ndcX, float ndcY, float screenWidth, float screenHeight)
    {
        return NDCToPixel(Vec2(ndcX, ndcY), screenWidth, screenHeight);
    }
}