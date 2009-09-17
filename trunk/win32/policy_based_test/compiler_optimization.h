struct compiler_optimisation_traits_ebo_parent
{};

/* EBO - Empty Base Optimisation
*/

/** \brief The result of Case 1, and the intermediate parent for Cases 3 & 4.
*
* \ingroup group__library__utility
*
*       compiler_optimisation_traits_ebo_parent
*                |
*                V
*     compiler_optimisation_traits_ebo_thin_child
*
*/
struct compiler_optimisation_traits_ebo_thin_child
	: compiler_optimisation_traits_ebo_parent
{};

/** \brief The result of Case 2
*
* \ingroup group__library__utility
*
*       compiler_optimisation_traits_ebo_parent
*                |
*                V
*     compiler_optimisation_traits_ebo_thick_child
*
*/
struct compiler_optimisation_traits_ebo_thick_child
	: compiler_optimisation_traits_ebo_parent
{
	int i;
};

/** \brief Used in Cases 2, 4, 6 & 8
*
* \ingroup group__library__utility
*
*/
struct compiler_optimisation_traits_ebo_thick_peer
{
	int i;
};

/** \brief The result of Case 3
*
* \ingroup group__library__utility
*
*       compiler_optimisation_traits_ebo_parent
*                |
*                V
*     compiler_optimisation_traits_ebo_thin_child
*                |
*                V
*   compiler_optimisation_traits_ebo_thin_grandchild
*
*/
struct compiler_optimisation_traits_ebo_thin_grandchild
	: compiler_optimisation_traits_ebo_thin_child
{};

/** \brief The result of Case 4
*
* \ingroup group__library__utility
*
*       compiler_optimisation_traits_ebo_parent
*                |
*                V
*     compiler_optimisation_traits_ebo_thin_child
*                |
*                V
*   compiler_optimisation_traits_ebo_thick_grandchild
*
*/
struct compiler_optimisation_traits_ebo_thick_grandchild
	: compiler_optimisation_traits_ebo_thin_child
{
	int i;
};

/** \brief Used in Case 5
*
* \ingroup group__library__utility
*
*/
struct compiler_optimisation_traits_ebo_parent2
{};

/** \brief The result of Case 5
*
* \ingroup group__library__utility
*
*   compiler_optimisation_traits_ebo_parent      compiler_optimisation_traits_ebo_parent2
*                |                        |
*                V                        V
*            compiler_optimisation_traits_ebo_mi_thin_child
*
*/
struct compiler_optimisation_traits_ebo_mi_thin_child
	: compiler_optimisation_traits_ebo_parent, compiler_optimisation_traits_ebo_parent2
{};

/** \brief The result of Case 6
*
* \ingroup group__library__utility
*
*   compiler_optimisation_traits_ebo_parent      compiler_optimisation_traits_ebo_parent2
*                |                        |
*                V                        V
*            compiler_optimisation_traits_ebo_mi_thick_child
*
*/
struct compiler_optimisation_traits_ebo_mi_thick_child
	: compiler_optimisation_traits_ebo_parent, compiler_optimisation_traits_ebo_parent2
{
	int i;
};

/** \brief The result of Case 7
*
* \ingroup group__library__utility
*
*   compiler_optimisation_traits_ebo_parent      compiler_optimisation_traits_ebo_thick_peer
*                |                        |
*                V                        V
*            compiler_optimisation_traits_ebo_mi_mixin_child
*
*/
struct compiler_optimisation_traits_ebo_mi_mixin_child
	: compiler_optimisation_traits_ebo_parent, compiler_optimisation_traits_ebo_thick_peer
{};


/* EDO - Empty Derived Optimisation
*/

/** \brief Used in Case 1
*
* \ingroup group__library__utility
*
*/
struct compiler_optimisation_traits_edo_thin_base
{};

/** \brief Used in Case 2
*
* \ingroup group__library__utility
*
*/
struct compiler_optimisation_traits_edo_thick_base
{
	int i;
};

/** \brief Used in Case 3
*
* \ingroup group__library__utility
*
*/
struct compiler_optimisation_traits_edo_thin_base2
{};

/** \brief Used in Cases 3 & 4
*
* \ingroup group__library__utility
*
*/
template <typename T>
struct compiler_optimisation_traits_edo_child
	: T
{};

/** \brief Result of case 3
*
* \ingroup group__library__utility
*
*       compiler_optimisation_traits_edo_thick_base
*                    |
*                    V
*    compiler_optimisation_traits_edo_child_of_thick_base
*
*/
struct compiler_optimisation_traits_edo_child_of_thick_base
	: compiler_optimisation_traits_edo_thick_base
{};


/** \brief Used in Cases 7 & 8
*
* \ingroup group__library__utility
*
*/
template<   typename  T1
,   typename  T2
>
struct compiler_optimisation_traits_edo_mi_child
	: T1, T2
{};

/** \brief traits class for (current) compiler
*
* \ingroup group__library__utility
*
*/
struct compiler_optimisation_traits
{
public:
	// Empty Base Optimisation (EBO)
	//
	/// 1. With empty base and empty child
	enum { supportsEBO1 = sizeof(compiler_optimisation_traits_ebo_thin_child) == sizeof(compiler_optimisation_traits_ebo_parent) };
	/// 2. With empty base and non-empty child
	enum { supportsEBO2 = sizeof(compiler_optimisation_traits_ebo_thick_child) == sizeof(compiler_optimisation_traits_ebo_thick_peer) };
	/// 3. With empty base, empty intermediate, and empty child
	enum { supportsEBO3 = sizeof(compiler_optimisation_traits_ebo_thin_grandchild) == sizeof(compiler_optimisation_traits_ebo_parent) };
	/// 4. With empty base, empty intermediate, and non-empty child
	enum { supportsEBO4 = sizeof(compiler_optimisation_traits_ebo_thick_grandchild) == sizeof(compiler_optimisation_traits_ebo_thick_peer) };
	/// 5. With two empty bases and empty child
	enum { supportsEBO5 = sizeof(compiler_optimisation_traits_ebo_mi_thin_child) == sizeof(compiler_optimisation_traits_ebo_parent) };
	/// 6. With two empty bases and non-empty child
	enum { supportsEBO6 = sizeof(compiler_optimisation_traits_ebo_mi_thick_child) == sizeof(compiler_optimisation_traits_ebo_thick_peer) };
	/// 7. With one empty base (the mixin), one non-empty base and empty child
	enum { supportsEBO7 = sizeof(compiler_optimisation_traits_ebo_mi_mixin_child) == sizeof(compiler_optimisation_traits_ebo_thick_peer) };

	enum { supportsEBO = supportsEBO1 & supportsEBO2 & supportsEBO3 & supportsEBO4 };
	enum { supportsMIEBO = supportsEBO5 & supportsEBO6 & supportsEBO7 /* & supportsEBO8 */ };
	enum { supportsExtendedEBO = supportsEBO & supportsMIEBO };

	// Empty Derived Optimisation (EDO)
	//
	/// 1. With empty base (same as EBO1)
	enum { supportsEDO1 = supportsEBO1 };
	/// 2. With empty base (same as EBO1); child is template
	enum { supportsEDO2 = sizeof(compiler_optimisation_traits_edo_child<compiler_optimisation_traits_edo_thin_base>) == sizeof(compiler_optimisation_traits_edo_thin_base) };
	/// 3. With non-empty base
	enum { supportsEDO3 = sizeof(compiler_optimisation_traits_edo_child_of_thick_base) == sizeof(compiler_optimisation_traits_edo_thick_base) };
	/// 4. With non-empty base; child is template
	enum { supportsEDO4 = sizeof(compiler_optimisation_traits_edo_child<compiler_optimisation_traits_edo_thick_base>) == sizeof(compiler_optimisation_traits_edo_thick_base) };

	/// 5. With two empty bases
	enum { supportsEDO5 = supportsEBO5 };
	/// 6. With two empty bases; child is template
	enum { supportsEDO6 = sizeof(compiler_optimisation_traits_edo_mi_child<compiler_optimisation_traits_edo_thin_base, compiler_optimisation_traits_edo_thin_base2>) == sizeof(compiler_optimisation_traits_edo_thin_base) };
	/// 7. With one empty base (the mixin), one non-empty base
	enum { supportsEDO7 = supportsEBO7 };
	/// 8. With one empty base (the mixin), one non-empty base; child is template
	enum { supportsEDO8 = sizeof(compiler_optimisation_traits_edo_mi_child<compiler_optimisation_traits_edo_thick_base, compiler_optimisation_traits_edo_thin_base>) == sizeof(compiler_optimisation_traits_edo_thick_base) };

	enum { supportsEDO = supportsEDO1 & supportsEDO2 & supportsEDO3 & supportsEDO4 };
	enum { supportsMIEDO = supportsEDO5 & supportsEDO6 & supportsEDO7 & supportsEDO8 };
	enum { supportsExtendedEDO = supportsEDO & supportsMIEDO };
};

/* ////////////////////////////////////////////////////////////////////// */

