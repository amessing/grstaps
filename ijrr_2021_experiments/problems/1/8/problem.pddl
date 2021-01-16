(define (problem ijrr_8)
	(:domain ijrr)
	(:objects
		survivor0 survivor1 - survivor
		street22 street55 - street
		building11 building14 building17 building19 - building
	)
	(:init
		(atLocation survivor0 street55)
		(atLocation survivor1 building19)
		(onFire building11)
		(onFire building14)
		(underRubble street22)
		(underRubble building17)
		(needsRepair building11)
		(needsRepair building14)
		(needsRepair building17)
	)
	(:goal (and
		(atLocation survivor0 HOSPITAL)
		(atLocation survivor1 HOSPITAL)
		(not (underRubble street22))
		(not (needsRepair building11))
		(not (needsRepair building14))
		(not (needsRepair building17))
	))
	(:metric minimize total-time)
)
