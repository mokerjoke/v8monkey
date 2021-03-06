/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* ***** BEGIN LICENSE BLOCK *****
 * Version: MPL 1.1/GPL 2.0/LGPL 2.1
 *
 * The contents of this file are subject to the Mozilla Public License Version
 * 1.1 (the "License"); you may not use this file except in compliance with
 * the License. You may obtain a copy of the License at
 * http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
 * for the specific language governing rights and limitations under the
 * License.
 *
 * The Original Code is the Mozilla SVG project.
 *
 * The Initial Developer of the Original Code is
 * Crocodile Clips Ltd..
 * Portions created by the Initial Developer are Copyright (C) 2002
 * the Initial Developer. All Rights Reserved.
 *
 * Contributor(s):
 *   Alex Fritze <alex.fritze@crocodile-clips.com> (original author)
 *
 * Alternatively, the contents of this file may be used under the terms of
 * either of the GNU General Public License Version 2 or later (the "GPL"),
 * or the GNU Lesser General Public License Version 2.1 or later (the "LGPL"),
 * in which case the provisions of the GPL or the LGPL are applicable instead
 * of those above. If you wish to allow use of your version of this file only
 * under the terms of either the GPL or the LGPL, and not to allow others to
 * use your version of this file under the terms of the MPL, indicate your
 * decision by deleting the provisions above and replace them with the notice
 * and other provisions required by the GPL or the LGPL. If you do not delete
 * the provisions above, a recipient may use your version of this file under
 * the terms of any one of the MPL, the GPL or the LGPL.
 *
 * ***** END LICENSE BLOCK ***** */

#ifndef __NS_SVGGLYPHFRAME_H__
#define __NS_SVGGLYPHFRAME_H__

#include "nsSVGGeometryFrame.h"
#include "nsISVGGlyphFragmentNode.h"
#include "nsISVGChildFrame.h"
#include "nsSVGUtils.h"
#include "gfxContext.h"
#include "gfxFont.h"
#include "nsTextFragment.h"

class nsSVGTextFrame;
class nsSVGTextPathFrame;
class nsSVGGlyphFrame;
class CharacterIterator;
struct CharacterPosition;

typedef gfxFont::DrawMode DrawMode;

typedef nsSVGGeometryFrame nsSVGGlyphFrameBase;

class nsSVGGlyphFrame : public nsSVGGlyphFrameBase,
                        public nsISVGGlyphFragmentNode,
                        public nsISVGChildFrame
{
  friend nsIFrame*
  NS_NewSVGGlyphFrame(nsIPresShell* aPresShell, nsStyleContext* aContext);
protected:
  nsSVGGlyphFrame(nsStyleContext* aContext)
    : nsSVGGlyphFrameBase(aContext),
      mTextRun(nsnull),
      mStartIndex(0),
      mCompressWhitespace(true),
      mTrimLeadingWhitespace(false),
      mTrimTrailingWhitespace(false)
      {}
  ~nsSVGGlyphFrame()
  {
    ClearTextRun();
  }

public:
  NS_DECL_QUERYFRAME
  NS_DECL_FRAMEARENA_HELPERS

  // These do not use the global transform if NS_STATE_NONDISPLAY_CHILD
  nsresult GetStartPositionOfChar(PRUint32 charnum, nsIDOMSVGPoint **_retval);
  nsresult GetEndPositionOfChar(PRUint32 charnum, nsIDOMSVGPoint **_retval);
  nsresult GetExtentOfChar(PRUint32 charnum, nsIDOMSVGRect **_retval);
  nsresult GetRotationOfChar(PRUint32 charnum, float *_retval);
  /**
   * @param aForceGlobalTransform controls whether to use the
   * global transform even when NS_STATE_NONDISPLAY_CHILD
   */
  float GetAdvance(bool aForceGlobalTransform);

  void SetGlyphPosition(gfxPoint *aPosition, bool aForceGlobalTransform);
  nsSVGTextPathFrame* FindTextPathParent();
  bool IsStartOfChunk(); // == is new absolutely positioned chunk.

  void GetXY(mozilla::SVGUserUnitList *aX, mozilla::SVGUserUnitList *aY);
  void SetStartIndex(PRUint32 aStartIndex);
  /*
   * Returns inherited x and y values instead of parent element's attribute
   * values.
   */
  void GetEffectiveXY(PRInt32 strLength,
                      nsTArray<float> &aX, nsTArray<float> &aY);
  /*
   * Returns inherited dx and dy values instead of parent element's attribute
   * values.
   */
  void GetEffectiveDxDy(PRInt32 strLength, 
                        nsTArray<float> &aDx,
                        nsTArray<float> &aDy);
  /*
   * Returns inherited rotate values instead of parent element's attribute
   * values.
   */
  void GetEffectiveRotate(PRInt32 strLength,
                          nsTArray<float> &aRotate);
  PRUint16 GetTextAnchor();
  bool IsAbsolutelyPositioned();
  bool IsTextEmpty() const {
    return mContent->GetText()->GetLength() == 0;
  }
  void SetTrimLeadingWhitespace(bool aTrimLeadingWhitespace) {
    if (mTrimLeadingWhitespace != aTrimLeadingWhitespace) {
      mTrimLeadingWhitespace = aTrimLeadingWhitespace;
      ClearTextRun();
    }
  }
  void SetTrimTrailingWhitespace(bool aTrimTrailingWhitespace) {
    if (mTrimTrailingWhitespace != aTrimTrailingWhitespace) {
      mTrimTrailingWhitespace = aTrimTrailingWhitespace;
      ClearTextRun();
    }
  }
  bool EndsWithWhitespace() const;
  bool IsAllWhitespace() const;

  // nsIFrame interface:
  NS_IMETHOD  CharacterDataChanged(CharacterDataChangeInfo* aInfo);

  virtual void DidSetStyleContext(nsStyleContext* aOldStyleContext);

  NS_IMETHOD  IsSelectable(bool* aIsSelectable, PRUint8* aSelectStyle) const;

  NS_IMETHOD Init(nsIContent*      aContent,
                  nsIFrame*        aParent,
                  nsIFrame*        aPrevInFlow);

  /**
   * Get the "type" of the frame
   *
   * @see nsGkAtoms::svgGlyphFrame
   */
  virtual nsIAtom* GetType() const;

  virtual bool IsFrameOfType(PRUint32 aFlags) const
  {
    // Set the frame state bit for text frames to mark them as replaced.
    // XXX kipp: temporary

    return nsSVGGlyphFrameBase::IsFrameOfType(aFlags & ~(nsIFrame::eReplaced));
  }

#ifdef DEBUG
  NS_IMETHOD GetFrameName(nsAString& aResult) const
  {
    return MakeFrameName(NS_LITERAL_STRING("SVGGlyph"), aResult);
  }
#endif

  // nsISVGChildFrame interface:
  // These four always use the global transform, even if NS_STATE_NONDISPLAY_CHILD
  NS_IMETHOD PaintSVG(nsSVGRenderState *aContext,
                      const nsIntRect *aDirtyRect);
  NS_IMETHOD_(nsIFrame*) GetFrameForPoint(const nsPoint &aPoint);
  NS_IMETHOD UpdateCoveredRegion();
  virtual gfxRect GetBBoxContribution(const gfxMatrix &aToBBoxUserspace,
                                      PRUint32 aFlags);

  NS_IMETHOD_(nsRect) GetCoveredRegion();
  NS_IMETHOD InitialUpdate();
  virtual void NotifySVGChanged(PRUint32 aFlags);
  virtual void NotifyRedrawSuspended();
  virtual void NotifyRedrawUnsuspended();
  NS_IMETHOD_(bool) IsDisplayContainer() { return false; }
  NS_IMETHOD_(bool) HasValidCoveredRect() {
    return !(GetStateBits() & NS_STATE_SVG_NONDISPLAY_CHILD);
  }

  // nsSVGGeometryFrame methods
  gfxMatrix GetCanvasTM();

  // nsISVGGlyphFragmentNode interface:
  // These do not use the global transform if NS_STATE_NONDISPLAY_CHILD
  virtual PRUint32 GetNumberOfChars();
  virtual float GetComputedTextLength();
  virtual float GetSubStringLength(PRUint32 charnum, PRUint32 fragmentChars);
  virtual PRInt32 GetCharNumAtPosition(nsIDOMSVGPoint *point);
  NS_IMETHOD_(nsSVGGlyphFrame *) GetFirstGlyphFrame();
  NS_IMETHOD_(nsSVGGlyphFrame *) GetNextGlyphFrame();
  NS_IMETHOD_(void) SetWhitespaceCompression(bool aCompressWhitespace) {
    if (mCompressWhitespace != aCompressWhitespace) {
      mCompressWhitespace = aCompressWhitespace;
      ClearTextRun();
    }
  }

protected:
  friend class CharacterIterator;

  // Use a power of 2 here. It's not so important to match
  // nsDeviceContext::AppUnitsPerDevPixel, but since we do a lot of
  // multiplying by 1/GetTextRunUnitsFactor, it's good for it to be a
  // power of 2 to avoid accuracy loss.
  static PRUint32 GetTextRunUnitsFactor() { return 64; }
  
  /**
   * @aParam aDrawScale font drawing must be scaled into user units
   * by this factor
   * @param aMetricsScale font metrics must be scaled into user units
   * by this factor
   * @param aForceGlobalTransform set to true if we should force use of
   * the global transform; otherwise we won't use the global transform
   * if we're a NONDISPLAY_CHILD
   */
  bool EnsureTextRun(float *aDrawScale, float *aMetricsScale,
                       bool aForceGlobalTransform);
  void ClearTextRun();

  bool GetCharacterData(nsAString & aCharacterData);
  bool GetCharacterPositions(nsTArray<CharacterPosition>* aCharacterPositions,
                               float aMetricsScale);
  PRUint32 GetTextRunFlags(PRUint32 strLength);

  void AddCharactersToPath(CharacterIterator *aIter,
                           gfxContext *aContext);
  void AddBoundingBoxesToPath(CharacterIterator *aIter,
                              gfxContext *aContext);
  void DrawCharacters(CharacterIterator *aIter,
                      gfxContext *aContext,
                      DrawMode aDrawMode,
                      gfxPattern *aStrokePattern = nsnull);

  void NotifyGlyphMetricsChange();
  void SetupGlobalTransform(gfxContext *aContext);
  nsresult GetHighlight(PRUint32 *charnum, PRUint32 *nchars,
                        nscolor *foreground, nscolor *background);
  float GetSubStringAdvance(PRUint32 charnum, PRUint32 fragmentChars,
                            float aMetricsScale);
  gfxFloat GetBaselineOffset(float aMetricsScale);

  virtual void GetDxDy(SVGUserUnitList *aDx, SVGUserUnitList *aDy);
  virtual const SVGNumberList *GetRotate();

  // Used to support GetBBoxContribution by making GetConvasTM use this as the
  // parent transform instead of the real CanvasTM.
  nsAutoPtr<gfxMatrix> mOverrideCanvasTM;

  // Owning pointer, must call gfxTextRunWordCache::RemoveTextRun before deleting
  gfxTextRun *mTextRun;
  gfxPoint mPosition;
  // The start index into the position and rotation data
  PRUint32 mStartIndex;
  bool mCompressWhitespace;
  bool mTrimLeadingWhitespace;
  bool mTrimTrailingWhitespace;

private:
  DrawMode SetupCairoState(gfxContext *context, nsRefPtr<gfxPattern> *strokePattern);
};

#endif
