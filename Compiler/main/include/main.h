#pragma once

const char* const grammar =
"Program\n"
"	: StatementList		$$ = $1\n"
"\n"
"PrimaryExpression\n"
"	: number				$$ = constant($1)\n"
"	| true					$$ = make(\"true\")\n"
"	| false					$$ = make(\"false\")\n"
"	| Identifier			$$ = $1\n"
"	| ( Expression )		$$ = $2\n"
"\n"
"PostfixExpression\n"
"	: PrimaryExpression							$$ = $1\n"
"	| PostfixExpression [ IntegerExpression ]	$$ = make(\"pe[i]\", $4, $2)\n"
"	| FunctionCall								$$ = $1\n"
"	| PostfixExpression . Identifier			$$ = make(\"p.id\", $3, $1)\n"
"	| PostfixExpression ++						$$ = make(\"v++\", $2)\n"
"	| PostfixExpression --						$$ = make(\"v--\", $2)\n"
"\n"
"IntegerExpression\n"
"	: Expression	$$ = $1\n"
"\n"
"FunctionCall\n"
"	: FunctionCallOrMethod	$$ = $1\n"
"\n"
"FunctionCallOrMethod\n"
"	: FunctionCallGeneric						$$ = $1\n"
"	| PostfixExpression . FunctionCallGeneric	$$ = make(\"pe.fcg\", $3, $1)\n"
"\n"
"FunctionCallGeneric\n"
"	: FunctionCallHeaderWithParameters )	$$ = $2\n"
"	| FunctionCallHeaderNoParameters )		$$ = $2\n"
"\n"
"FunctionCallHeaderNoParameters\n"
//"	: FunctionCallHeader void	$$ = $2\n"
"	: FunctionCallHeader		$$ = $1\n"
"\n"
"FunctionCallHeaderWithParameters\n"
"	: FunctionCallHeader AssignmentExpression					$$ = make(\"fc_with_p\", $2, $1)\n"
"	| FunctionCallHeaderWithParameters , AssignmentExpression	$$ = make(\"fc_with_p2\", $3, $1)\n"
"\n"
"FunctionCallHeader\n"
"	: FunctionIdentifier (		$$ = $1\n"
"\n"
"FunctionIdentifier\n"
"	: TypeSpecifier		$$ = $1\n"
"	| Identifier		$$ = $1\n"
"\n"
"UnaryExpression\n"
"	: PostfixExpression					$$ = $1\n"
"	| ++ UnaryExpression				$$ = make(\"++v\", $1)\n"
"	| -- UnaryExpression				$$ = make(\"--v\", $1)\n"
"	| UnaryOperator UnaryExpression		$$ = make(\"up_ue\", $2, $1)\n"
"\n"
"UnaryOperator\n"
"	: +		$$ = make(\"+u\")\n"
"	| -		$$ = make(\"-u\")\n"
"	| !		$$ = make(\"!u\")\n"
"	| ~		$$ = make(\"~u\")\n"
"\n"
"MultiplicativeExpression\n"
"	: UnaryExpression								$$ = $1\n"
"	| MultiplicativeExpression * UnaryExpression	$$ = make(\"*\", $3, $1)\n"
"	| MultiplicativeExpression / UnaryExpression	$$ = make(\"/\", $3, $1)\n"
"	| MultiplicativeExpression % UnaryExpression	$$ = make(\"%\", $3, $1)\n"
"\n"
"AdditiveExpression\n"
"	: MultiplicativeExpression						$$ = $1\n"
"	| AdditiveExpression + MultiplicativeExpression	$$ = make(\"+\", $3, $1)\n"
"	| AdditiveExpression - MultiplicativeExpression	$$ = make(\"-\", $3, $1)\n"
"\n"
"ShiftExpression\n"
"	: AdditiveExpression					$$ = $1\n"
"	| ShiftExpression << AdditiveExpression	$$ = make(\"<<\", $3, $1)\n"
"	| ShiftExpression >> AdditiveExpression	$$ = make(\">>\", $3, $1)\n"
"\n"
"RelationalExpression\n"
"	: ShiftExpression							$$ = $1\n"
"	| RelationalExpression < ShiftExpression	$$ = make(\"<\", $3, $1)\n"
"	| RelationalExpression > ShiftExpression	$$ = make(\"<\", $3, $1)\n"
"	| RelationalExpression <= ShiftExpression	$$ = make(\"<=\", $3, $1)\n"
"	| RelationalExpression >= ShiftExpression	$$ = make(\">=\", $3, $1)\n"
"\n"
"EqualityExpression\n"
"	: RelationalExpression							$$ = $1\n"
"	| EqualityExpression == RelationalExpression	$$ = make(\"==\", $3, $1)\n"
"	| EqualityExpression != RelationalExpression	$$ = make(\"!=\", $3, $1)\n"
"\n"
"AndExpression\n"
"	: EqualityExpression					$$ = $1\n"
"	| AndExpression & EqualityExpression	$$ = make(\"&\", $3, $1)\n"
"\n"
"ExclusiveOrExpression\n"
"	: AndExpression							$$ = $1\n"
"	| ExclusiveOrExpression ^ AndExpression	$$ = make(\"^\", $3, $1)\n"
"\n"
"InclusiveOrExpression\n"
"	: ExclusiveOrExpression							$$ = $1\n"
"	| InclusiveOrExpression | ExclusiveOrExpression	$$ = make(\"|\", $3, $1)\n"
"\n"
"LogicalAndExpression\n"
"	: InclusiveOrExpression							$$ = $1\n"
"	| LogicalAndExpression && InclusiveOrExpression	$$ = make(\"&&\", $3, $1)\n"
"\n"
"LogicalXorExpression\n"
"	: LogicalAndExpression							$$ = $1\n"
"	| LogicalXorExpression ^^ LogicalAndExpression	$$ = make(\"^^\", $3, $1)\n"
"\n"
"LogicalOrExpression\n"
"	: LogicalXorExpression							$$ = $1\n"
"	| LogicalOrExpression || LogicalXorExpression	$$ = make(\"||\", $3, $1)\n"
"\n"
"ConditionalExpression\n"
"	: LogicalOrExpression										$$ = $1\n"
"	| LogicalOrExpression ? Expression : AssignmentExpression	$$ = make(\"?:\", $5, $3, $1)\n"
"\n"
"AssignmentExpression\n"
"	: ConditionalExpression										$$ = $1\n"
"	| UnaryExpression AssignmentOperator AssignmentExpression	$$ = make(\"assignment\", $3, $2, $1)\n"
"\n"
"AssignmentOperator\n"
"	: =		$$ = make(\"=\")\n"
"	| *=	$$ = make(\"*=\")\n"
"	| /=	$$ = make(\"/=\")\n"
"	| %=	$$ = make(\"%=\")\n"
"	| +=	$$ = make(\"+=\")\n"
"	| -=	$$ = make(\"-=\")\n"
"	| <<=	$$ = make(\"<<=\")\n"
"	| >>=	$$ = make(\">>=\")\n"
"	| &=	$$ = make(\"&=\")\n"
"	| ^=	$$ = make(\"^=\")\n"
"	| |=	$$ = make(\"|=\")\n"
"\n"
"Expression\n"
"	: AssignmentExpression				$$ = $1\n"
"	| Expression , AssignmentExpression	$$ = make(\"expression\", $3, $1)\n"
"\n"
"ConstantExpression\n"
"	: ConditionalExpression		$$ = $1\n"
"\n"
"Declaration\n"
"	: FunctionPrototype ;																		$$ = $2\n"
"	| InitDeclaratorList ;																		$$ = $2\n"
//"	| precision PrecisionQualifier TypeSpecifierNoPrec ;										$$ = make(\"precision_decl\", $3, $2)\n"
"	| TypeQualifier Identifier { StructDeclarationList } ;										$$ = make(\"full_decl\", $6, $5, $3, $0, $0)\n"
"	| TypeQualifier Identifier { StructDeclarationList } Identifier ;							$$ = make(\"full_decl\", $7, $6, $4, $2, $0)\n"
"	| TypeQualifier Identifier { StructDeclarationList } Identifier [ ] ;						$$ = make(\"full_decl\", $9, $8, $6, $4, $0)\n"
"	| TypeQualifier Identifier { StructDeclarationList } Identifier [ ConstantExpression ] ;	$$ = make(\"full_decl\", $10, $9, $7, $5, $3)\n"
"	| TypeQualifier ;																			$$ = $2\n"
"\n"
"FunctionPrototype\n"
"	: FunctionDeclarator )	$$ = $2\n"
"\n"
"FunctionDeclarator\n"
"	: FunctionHeader				$$ = $1\n"
"	| FunctionHeaderWithParameters	$$ = $1\n"
"\n"
"FunctionHeaderWithParameters\n"
"	: FunctionHeader ParameterDeclaration					$$ = make(\"func_header_with_param\", $2, $1)\n"
"	| FunctionHeaderWithParameters , ParameterDeclaration	$$ = make(\"func_header_with_param2\", $3, $1)\n"
"\n"
"FunctionHeader\n"
"	: FullySpecifiedType Identifier (	$$ = make(\"func_header\", $3, $2)\n"
"\n"
"ParameterDeclarator\n"
"	: TypeSpecifier Identifier							$$ = make(\"param_declr\", $2, $1, $0)\n"
"	| TypeSpecifier Identifier [ ConstantExpression ]	$$ = make(\"param_declr\", $5, $4, $2)\n"
"\n"
"ParameterDeclaration\n"
"	: ParameterTypeQualifier ParameterQualifier ParameterDeclarator		$$ = make(\"param_decln\", $3, $2, $0, $1)\n"
"	| ParameterQualifier ParameterDeclarator							$$ = make(\"param_decln\", $0, $2, $0, $1)\n"
"	| ParameterTypeQualifier ParameterQualifier ParameterTypeSpecifier	$$ = make(\"param_decln\", $3, $2, $1, $0)\n"
"	| ParameterQualifier ParameterTypeSpecifier							$$ = make(\"param_decln\", $0, $2, $1, $0)\n"
"\n"
"ParameterQualifier\n"
"	: epsilon	$$ = $0\n"
"	| in		$$ = make(\"in\")\n"
"	| out		$$ = make(\"out\")\n"
"	| inout		$$ = make(\"inout\")\n"
"\n"
"ParameterTypeSpecifier\n"
"	: TypeSpecifier		$$ = $1\n"
"\n"
"InitDeclaratorList\n"
"	: SingleDeclaration														$$ = $1\n"
"	| InitDeclaratorList , Identifier										$$ = make(\"init_declr_list\", $3, $1)\n"
"	| InitDeclaratorList , Identifier [ ]									$$ = make(\"init_declr_list2\", $5, $3)\n"
"	| InitDeclaratorList , Identifier [ ConstantExpression ]				$$ = make(\"init_declr_list3\", $6, $4, $2)\n"
"	| InitDeclaratorList , Identifier [ ] = Initializer						$$ = make(\"init_declr_list4\", $7, $5, $1)\n"
"	| InitDeclaratorList , Identifier [ ConstantExpression ] = Initializer	$$ = make(\"init_declr_list5\", $8, $6, $4, $1)\n"
"	| InitDeclaratorList , Identifier = Initializer							$$ = make(\"init_declr_list6\", $5, $3, $1)\n"
"\n"
"SingleDeclaration\n"
"	: FullySpecifiedType													$$ = $1\n"
"	| FullySpecifiedType Identifier											$$ = make(\"single_decln\", $2, $1)\n"
"	| FullySpecifiedType Identifier [ ]										$$ = make(\"single_decln2\", $4, $3)\n"
"	| FullySpecifiedType Identifier [ ConstantExpression ]					$$ = make(\"single_decln3\", $5, $4, $2)\n"
"	| FullySpecifiedType Identifier [ ] = Initializer						$$ = make(\"single_decln4\", $6, $5, $1)\n"
"	| FullySpecifiedType Identifier [ ConstantExpression ] = Initializer	$$ = make(\"single_decln5\", $7, $6, $4, $1)\n"
"	| FullySpecifiedType Identifier = Initializer							$$ = make(\"single_decln6\", $4, $4, $1)\n"
"	| invariant Identifier													$$ = $1\n"
"\n"
"FullySpecifiedType\n"
"	: TypeSpecifier					$$ = $1\n"
"	| TypeQualifier TypeSpecifier	$$ = make(\"fully_spec_t\", $2, $1)\n"
"\n"
"InvariantQualifier\n"
"	: invariant						$$ = make(\"invariant\")\n"
"\n"
"InterpolationQualifier\n"
"	: smooth			$$ = make(\"smooth\")\n"
"	| flat				$$ = make(\"flat\")\n"
"	| noperspective		$$ = make(\"noperspective\")\n"
"\n"
"LayoutQualifier\n"
"	: layout ( LayoutQualifierIdList )	$$ = $2\n"
"\n"
"LayoutQualifierIdList\n"
"	: LayoutQualifierId							$$ = $1\n"
"	| LayoutQualifierIdList , LayoutQualifierId	$$ = make(\"layout_qual_id_list\", $3, $1)\n"
"\n"
"LayoutQualifierId\n"
"	: Identifier			$$ = $1\n"
"	| Identifier = number	$$ = $3\n"
"\n"
"ParameterTypeQualifier\n"
"	: const					$$ = make(\"const\")\n"
"\n"
"TypeQualifier\n"
"	: StorageQualifier												$$ = $1\n"
"	| LayoutQualifier												$$ = $1\n"
"	| LayoutQualifier StorageQualifier								$$ = make(\"type_qual\", $2, $1)\n"
"	| InterpolationQualifier StorageQualifier						$$ = make(\"type_qual2\", $2, $1)\n"
"	| InterpolationQualifier										$$ = $1\n"
"	| InvariantQualifier StorageQualifier							$$ = make(\"type_qual3\", $2, $1)\n"
"	| InvariantQualifier InterpolationQualifier StorageQualifier	$$ = make(\"type_qual4\", $3, $2, $1)\n"
"	| invariant														$$ = make(\"invariant\")\n"
"\n"
"StorageQualifier\n"
"	: const				$$ = make(\"const_storage\")\n"
//"	| attribute			$$ = make(\"attribute\")\n"
//"	| varying			$$ = make(\"varying\")\n"
//"	| centroid varying	$$ = make(\"centroid_varying\")\n"
"	| in				$$ = make(\"in_storage\")\n"
"	| out				$$ = make(\"out_storage\")\n"
//"	| centroid in		$$ = make(\"centroid_in\")\n"
//"	| centroid out		$$ = make(\"centroid_out\")\n"
//"	| patch in			$$ = make(\"patch_in\")\n"
//"	| patch out			$$ = make(\"patch_out\")\n"
//"	| sample in			$$ = make(\"sample_in\")\n"
//"	| sample out		$$ = make(\"sample_out\")\n"
"	| uniform			$$ = make(\"uniform\")\n"
"\n"
"TypeSpecifier\n"
"	: TypeSpecifierNoPrec						$$ = $1\n"
//"	| PrecisionQualifier TypeSpecifierNoPrec	$$ = make(\"type_spec\", $2, $1)\n"
"\n"
"TypeSpecifierNoPrec\n"
"	: TypeSpecifierNonarray							$$ = $1\n"
"	| TypeSpecifierNonarray [ ]						$$ = make(\"type_spec_no_prec\", $3)\n"
"	| TypeSpecifierNonarray [ ConstantExpression ]	$$ = make(\"type_spec_no_prec2\", $4, $2)\n"
"\n"
"TypeSpecifierNonarray\n"
"	: void						$$ = make(\"void\")\n"
"	| float						$$ = make(\"float\")\n"
//"	| double					$$ = make(\"double\")\n"
"	| int						$$ = make(\"int\")\n"
"	| uint						$$ = make(\"uint\")\n"
"	| bool						$$ = make(\"bool\")\n"
"	| vec2						$$ = make(\"vec2\")\n"
"	| vec3						$$ = make(\"vec3\")\n"
"	| vec4						$$ = make(\"vec4\")\n"
/*"	| dvec2						$$ = make(\"dvec2\")\n"
"	| dvec3						$$ = make(\"dvec3\")\n"
"	| dvec4						$$ = make(\"dvec4\")\n"
"	| bvec2						$$ = make(\"bvec2\")\n"
"	| bvec3						$$ = make(\"bvec3\")\n"
"	| bvec4						$$ = make(\"bvec4\")\n"
"	| ivec2						$$ = make(\"ivec2\")\n"
"	| ivec3						$$ = make(\"ivec3\")\n"*/
"	| ivec4						$$ = make(\"ivec4\")\n"
/*"	| uvec2						$$ = make(\"uvec2\")\n"
"	| uvec3						$$ = make(\"uvec3\")\n"
"	| uvec4						$$ = make(\"uvec4\")\n"
"	| mat2						$$ = make(\"mat2\")\n"*/
"	| mat3						$$ = make(\"mat3\")\n"
"	| mat4						$$ = make(\"mat4\")\n"
/*"	| mat2x2					$$ = make(\"mat2x2\")\n"
"	| mat2x3					$$ = make(\"mat2x3\")\n"
"	| mat2x4					$$ = make(\"mat2x4\")\n"
"	| mat3x2					$$ = make(\"mat3x2\")\n"
"	| mat3x3					$$ = make(\"mat3x3\")\n"
"	| mat3x4					$$ = make(\"mat3x4\")\n"
"	| mat4x2					$$ = make(\"mat4x2\")\n"
"	| mat4x3					$$ = make(\"mat4x3\")\n"*/
"	| mat4						$$ = make(\"mat4\")\n"
/*"	| dmat2						$$ = make(\"dmat2\")\n"
"	| dmat3						$$ = make(\"dmat3\")\n"
"	| dmat4						$$ = make(\"dmat4\")\n"
"	| dmat2x2					$$ = make(\"dmat2x2\")\n"
"	| dmat2x3					$$ = make(\"dmat2x3\")\n"
"	| dmat2x4					$$ = make(\"dmat2x4\")\n"
"	| dmat3x2					$$ = make(\"dmat3x2\")\n"
"	| dmat3x3					$$ = make(\"dmat3x3\")\n"
"	| dmat3x4					$$ = make(\"dmat3x4\")\n"
"	| dmat4x2					$$ = make(\"dmat4x2\")\n"
"	| dmat4x3					$$ = make(\"dmat4x3\")\n"
"	| dmat4x4					$$ = make(\"dmat4x4\")\n"*/
//"	| sampler1D					$$ = make(\"sampler1D\")\n"
"	| sampler2D					$$ = make(\"sampler2D\")\n"
//"	| sampler3D					$$ = make(\"sampler3D\")\n"
"	| samplerCube				$$ = make(\"samplerCube\")\n"
//"	| sampler1DShadow			$$ = make(\"sampler1DShadow\")\n"
"	| sampler2DShadow			$$ = make(\"sampler2DShadow\")\n"
/*"	| samplerCubeShadow			$$ = make(\"samplerCubeShadow\")\n"
"	| sampler1DArray			$$ = make(\"sampler1DArray\")\n"
"	| sampler2DArray			$$ = make(\"sampler2DArray\")\n"
"	| sampler1DArrayShadow		$$ = make(\"sampler1DArrayShadow\")\n"
"	| sampler2DArrayShadow		$$ = make(\"sampler2DArrayShadow\")\n"
"	| samplerCubeArray			$$ = make(\"samplerCubeArray\")\n"
"	| samplerCubeArrayShadow	$$ = make(\"samplerCubeArrayShadow\")\n"
"	| isampler1D				$$ = make(\"isampler1D\")\n"
"	| isampler2D				$$ = make(\"isampler2D\")\n"
"	| isampler3D				$$ = make(\"isampler3D\")\n"
"	| isamplerCube				$$ = make(\"isamplerCube\")\n"
"	| isampler1DArray			$$ = make(\"isampler1DArray\")\n"
"	| isampler2DArray			$$ = make(\"isampler2DArray\")\n"
"	| isamplerCubeArray			$$ = make(\"isamplerCubeArray\")\n"
"	| usampler1D				$$ = make(\"usampler1D\")\n"
"	| usampler2D				$$ = make(\"usampler2D\")\n"
"	| usampler3D				$$ = make(\"usampler3D\")\n"
"	| usamplerCube				$$ = make(\"usamplerCube\")\n"
"	| usampler1DArray			$$ = make(\"usampler1DArray\")\n"
"	| usampler2DArray			$$ = make(\"usampler2DArray\")\n"
"	| usamplerCubeArray			$$ = make(\"usamplerCubeArray\")\n"
"	| sampler2DRect				$$ = make(\"sampler2DRect\")\n"
"	| sampler2DRectShadow		$$ = make(\"sampler2DRectShadow\")\n"
"	| isampler2DRect			$$ = make(\"isampler2DRect\")\n"
"	| usampler2DRect			$$ = make(\"usampler2DRect\")\n"
"	| samplerBuffer				$$ = make(\"samplerBuffer\")\n"
"	| isamplerBuffer			$$ = make(\"isamplerBuffer\")\n"
"	| usamplerBuffer			$$ = make(\"usamplerBuffer\")\n"
"	| sampler2DMS				$$ = make(\"sampler2DMS\")\n"
"	| isampler2DMS				$$ = make(\"isampler2DMS\")\n"
"	| usampler2DMS				$$ = make(\"usampler2DMS\")\n"
"	| sampler2DMSArray			$$ = make(\"sampler2DMSArray\")\n"
"	| isampler2DMSArray			$$ = make(\"isampler2DMSArray\")\n"
"	| usampler2DMSArray			$$ = make(\"usampler2DMSArray\")\n"*/
"	| StructSpecifier			$$ = $1\n"
"\n"
//"PrecisionQualifier\n"
//"	: highp		$$ = make(\"highp\")\n"
//"	| mediump	$$ = make(\"mediump\")\n"
//"	| lowp		$$ = make(\"lowp\")\n"
"\n"
"StructSpecifier\n"
"	: struct Identifier { StructDeclarationList }	$$ = make(\"struct\", $4, $2)\n"
//"	| struct { StructDeclarationList }				$$ = make(\"struct2\", $2)\n"
"\n"
"StructDeclarationList\n"
"	: StructDeclaration							$$ = $1\n"
"	| StructDeclarationList StructDeclaration	$$ = make(\"struct_decln_list\", $2, $1)\n"
"\n"
"StructDeclaration\n"
"	: TypeSpecifier StructDeclaratorList ;					$$ = make(\"struct_decln\", $3, $2)\n"
"	| TypeQualifier TypeSpecifier StructDeclaratorList ;	$$ = make(\"struct_decln2\", $4, $3, $2)\n"
"\n"
"StructDeclaratorList\n"
"	: StructDeclarator							$$ = $1\n"
"	| StructDeclaratorList , StructDeclarator	$$ = make(\"struct_declr_list\", $3, $1)\n"
"\n"
"StructDeclarator\n"
"	: Identifier						$$ = $1\n"
"	| Identifier [ ]					$$ = make(\"struct_delcr\", $3)\n"
"	| Identifier [ ConstantExpression ]	$$ = make(\"struct_delcr\", $4, $2)\n"
"\n"
"Initializer\n"
"	: AssignmentExpression	$$ = $1\n"
"\n"
"DeclarationStatement\n"
"	: Declaration		$$ = $1\n"
"\n"
"Statement\n"
"	: CompoundStatement	$$ = $1\n"
"	| SimpleStatement	$$ = $1\n"
"\n"
"SimpleStatement\n"
"	: DeclarationStatement	$$ = $1\n"
"	| ExpressionStatement	$$ = $1\n"
"	| SelectionStatement	$$ = $1\n"
"	| SwitchStatement		$$ = $1\n"
"	| CaseLabel				$$ = $1\n"
"	| IterationStatement	$$ = $1\n"
"	| JumpStatement			$$ = $1\n"
"\n"
"CompoundStatement\n"
"	: { }				$$ = make(\"comp_stat\")\n"
"	| { StatementList }	$$ = make(\"comp_stat2\", $2)\n"
"\n"
"StatementNoNewScope\n"
"	: CompoundStatementNoNewScope	$$ = $1\n"
"	| SimpleStatement				$$ = $1\n"
"\n"
"CompoundStatementNoNewScope\n"
"	: { }				$$ = make(\"comp_stat_no_new_scope\")\n"
"	| { StatementList }	$$ = make(\"comp_stat_no_new_scope\", $2)\n"
"\n"
"StatementList\n"
"	: Statement					$$ = $1\n"
"	| StatementList Statement	$$ = make(\"stat_list\", $2, $1)\n"
"\n"
"ExpressionStatement\n"
"	: ;				$$ = $0\n"
"	| Expression ;	$$ = $2\n"
"\n"
"SelectionStatement\n"
"	: if ( Expression ) SelectionRestStatement	$$ = make(\"sel\", $3, $1)\n"
"\n"
"SelectionRestStatement\n"
"	: Statement else Statement					$$ = make(\"sel_rest\", $3, $1)\n"
"	| Statement									$$ = $1\n"
"\n"
"Condition\n"
"	: Expression									$$ = $1\n"
"	| FullySpecifiedType Identifier = Initializer	$$ = make(\"condition\", $4, $3, $1)\n"
"\n"
"SwitchStatement\n"
"	: switch ( Expression ) { SwitchStatementList }	$$ = make(\"switch\", $5, $2)\n"
"\n"
"SwitchStatementList\n"
"	: epsilon		$$ = $0\n"
"	| StatementList	$$ = $1\n"
"\n"
"CaseLabel\n"
"	: case Expression :	$$ = make(\"case\", $2)\n"
"	| default :			$$ = make(\"default\")\n"
"\n"
"IterationStatement\n"
"	: while ( Condition ) StatementNoNewScope						$$ = make(\"while\", $3, $1)\n"
"	| do Statement while ( Expression ) ;							$$ = make(\"do_while\", $6, $3)\n"
"	| for ( ForInitStatement ForRestStatement ) StatementNoNewScope	$$ = make(\"for_loop\", $4, $3, $1)\n"
"\n"
"ForInitStatement\n"
"	: ExpressionStatement	$$ = $1\n"
"	| DeclarationStatement	$$ = $1\n"
"\n"
"Conditionopt\n"
"	: epsilon	$$ = $0\n"
"	| Condition	$$ = $1\n"
"\n"
"ForRestStatement\n"
"	: Conditionopt ;			$$ = $2\n"
"	| Conditionopt ; Expression	$$ = make(\"for_rest\", $3, $1)\n"
"\n"
"JumpStatement\n"
"	: continue ;			$$ = make(\"continue\")\n"
"	| break ;				$$ = make(\"break\")\n"
"	| return ;				$$ = make(\"return\")\n"
"	| return Expression ;	$$ = make(\"return_exp\", $2)\n"
"	| discard ;				$$ = make(\"discard\")\n"
"\n"
"TranslationUnit\n"
"	: ExternalDeclaration					$$ = $1\n"
"	| TranslationUnit ExternalDeclaration	$$ = make(\"trans_unit\", $2, $1)\n"
"\n"
"ExternalDeclaration\n"
"	: FunctionDefinition	$$ = $1\n"
"	| Declaration			$$ = $1\n"
"	| ;						$$ = $0\n"
"\n"
"FunctionDefinition\n"
"	: FunctionPrototype CompoundStatementNoNewScope	$$ = make(\"func_def\", $2, $1)\n"
"\n"
"Identifier\n"
"	identifier	$$ = symbol($1)\n"
"\n"
"String\n"
"	string 	$$ = literal($1)\n";