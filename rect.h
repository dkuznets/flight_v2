#ifndef	__RECT__H
#define	__RECT__H
//----------------------------------------------------------------------------

typedef __packed struct
{
	unsigned short left;
	unsigned short top;
	unsigned short right;
	unsigned short bottom;

} ARM_RECT;

//----------------------------------------------------------------------------

__inline void CopyRect(ARM_RECT *dst, const ARM_RECT *src)
{
	dst->left   = src->left;
	dst->top    = src->top;
	dst->right  = src->right;
	dst->bottom = src->bottom;
}

//----------------------------------------------------------------------------

__inline BIT EmptyRect(const ARM_RECT *r)
{
	return !(((S32)r->right - (S32)r->left) > 0 &&
		((S32)r->bottom - (S32)r->top) > 0);
}

//----------------------------------------------------------------------------

__inline BIT IntersectRect(ARM_RECT *r, const ARM_RECT *r1, const ARM_RECT *r2)
{
	ARM_RECT rect;

	CopyRect(&rect, r1);

	if(r2->left   > r1->left  ) rect.left   = r2->left;
	if(r2->top    > r1->top   ) rect.top    = r2->top;
	if(r2->right  < r1->right ) rect.right  = r2->right;
	if(r2->bottom < r1->bottom) rect.bottom = r2->bottom;

	if(r)
		*r = rect;

	return !EmptyRect(&rect);
}

//----------------------------------------------------------------------------
#endif	// __RECT__H
