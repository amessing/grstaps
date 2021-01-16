(define (problem ijrr_18)
	(:domain ijrr)
	(:objects
		survivor0 survivor1 - survivor
		street23 street52 - street
		building11 building22 building35 - building
	)
	(:init
		(atLocation survivor0 street52)
		(atLocation survivor1 building22)
		(onFire building11)
		(onFire building35)
		(underRubble street23)
		(underRubble building35)
		(needsRepair building11)
		(needsRepair building35)
	)
	(:goal (and
		(atLocation survivor0 HOSPITAL)
		(atLocation survivor1 HOSPITAL)
		(not (underRubble street23))
		(not (needsRepair building11))
		(not (needsRepair building35))
	))
	(:metric minimize total-time)
)
